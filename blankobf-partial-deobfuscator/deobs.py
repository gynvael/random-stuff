#!/usr/bin/env python3
# I was bored and made an AST-transformer based deobfuscator for some malware
# which is said to have been obfuscated with https://github.com/Blank-c/BlankOBF
# Note that I haven't looked at all at BlankOBF, just played with this malware.
# -- Gynvael Coldwind (Dec'23)
import ast
import sys
import base64

class BytesListTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if isinstance(node.func, ast.Name) and node.func.id == 'bytes':
      if len(node.args) == 1 and isinstance(node.args[0], ast.List):
        if all(isinstance(elem, ast.Constant) and isinstance(elem.value, int)
               for elem in node.args[0].elts):
          byte_values = bytes([elem.value for elem in node.args[0].elts])
          new_node = ast.Bytes(s=byte_values)
          return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class BytesDecodeTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Attribute) and
        node.func.attr == 'decode' and
        isinstance(node.func.value, ast.Constant) and
        isinstance(node.func.value.value, bytes)
      ):
        new_node = ast.Str(s=node.func.value.value.decode())
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class ImportTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Name) and
        node.func.id == '__import__' and
        len(node.args) == 1 and
        isinstance(node.args[0], ast.Constant) and
        isinstance(node.args[0].value, str)
      ):
        new_node = ast.Name(id=node.args[0].value)
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class GetattrTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Name) and
        node.func.id == 'getattr' and
        len(node.args) == 2 and
        isinstance(node.args[0], ast.Name) and
        isinstance(node.args[1], ast.Constant) and
        isinstance(node.args[1].value, str)
      ):
        new_node = ast.Attribute(value=node.args[0], attr=node.args[1].value)
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class BuiltinsTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Attribute) and
        isinstance(node.func.value, ast.Name) and
        node.func.value.id == 'builtins'
      ):
        new_node = ast.Call(
            func=ast.Name(id=node.func.attr),
            args=node.args,
            keywords=node.keywords
        )
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class Base64Transformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Attribute) and
        node.func.attr == 'b64decode' and
        isinstance(node.func.value, ast.Name) and
        node.func.value.id == 'base64' and
        len(node.args) == 1 and
        isinstance(node.args[0], ast.Constant) and
        (isinstance(node.args[0].value, str) or
         isinstance(node.args[0].value, bytes))
      ):
        new_node = ast.Bytes(s=base64.b64decode(node.args[0].value))
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)


class SimpleEvalTransformer(ast.NodeTransformer):
  def visit_Call(self, node):
    if (isinstance(node.func, ast.Name) and
        node.func.id == 'eval' and
        len(node.args) == 1 and
        isinstance(node.args[0], ast.Constant) and
        isinstance(node.args[0].value, str)
      ):
        print("# Eval of:", node.args[0].value)
        src = node.args[0].value
        new_node = ast.Name(id=deobf(src))
        return ast.copy_location(new_node, node)

    return self.generic_visit(node)

class SimpleAssignmentTransformer(ast.NodeTransformer):
  def __init__(self, alias_map):
    super().__init__()
    self.alias_map = alias_map

  def visit_Assign(self, node):
    if (len(node.targets) == 1 and
        isinstance(node.targets[0], ast.Name) and
        isinstance(node.value, ast.Name) and
        isinstance(node.value.id, str) and
        len(set(node.targets[0].id)) == 1 and
        node.targets[0].id[0] == '_'
      ):

      alias = node.targets[0].id
      alias_of = node.value.id

      #print("ALIAS FOUND:", alias, "is", alias_of)

      if alias in self.alias_map:
        print(f"# WARNING: alias \"{alias}\" of \"{alias_of}\" already in map!")

      self.alias_map[alias] = alias_of

    return self.generic_visit(node)

class AliasTransformer(ast.NodeTransformer):
  def __init__(self, alias_map):
    super().__init__()
    self.alias_map = alias_map

  def visit_Name(self, node):
    alias_of = self.alias_map.get(node.id)
    if alias_of is None:
      return self.generic_visit(node)

    new_node = ast.Name(id=alias_of)
    return ast.copy_location(new_node, node)


def deobf(src):
  alias_map = {}
  tree = ast.parse(src)

  # Perform transformations.
  old_code = ast.unparse(tree)
  it = 1
  while True:
    print(f"# Transformation iteration {it}")
    for trans in [
        BytesListTransformer(),
        BytesDecodeTransformer(),
        ImportTransformer(),
        GetattrTransformer(),
        Base64Transformer(),
        SimpleEvalTransformer(),
        BuiltinsTransformer(),
        SimpleAssignmentTransformer(alias_map),
        AliasTransformer(alias_map)
    ]:
      tree = trans.visit(tree)

    new_code = ast.unparse(tree)
    if new_code == old_code:
      break
    old_code = new_code
    it += 1

  #print(ast.dump(tree, indent=2))

  return ast.unparse(tree)

def main():
  if len(sys.argv) != 2:
    sys.exit("usage: deobs.py <fname.py>")

  with open(sys.argv[1]) as f:
    src = f.read()

  print(deobf(src))



if __name__ == "__main__":
  main()
