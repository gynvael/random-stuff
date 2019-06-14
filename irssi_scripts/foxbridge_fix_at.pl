$VERSION= '0.00.02';
%IRSSI = (
  authors => 'Gynvael Coldwind',
  contact => 'gynvael@coldwind.pl',
  name => 'foxbridge_fix',
  description => 'Discord bridge utilities, e.g. displaying messages in a better way.',
  license => 'BSD style',
  url => 'https://gynvael.coldwind.pl/'
);

# Special thanks to wiechu!

sub handle_message {
  my ($server, $msg, $nick, $nick_addr, $channel, @rest) = @_;

  if ($nick ne 'foxbridge') {
    return;
  }

  if (($channel ne '#gynvaelstream') && ($channel ne '#gynvaelstream-en')) {
    return;
  }

  #my  $string = $msg;
  #$string =~ s/(.)/sprintf("%.2x",ord($1))/eg;
  #print $msg;
  #print $string;
  if ($msg =~ s{^<\x03(0?[02-9]|1[0-5])(.+?)\x0f>\s}{}x) {
    $nick = '@' . $2;
    Irssi::signal_continue($server, $msg, $nick, $nick_addr, $channel, @rest);
  }
}

Irssi::signal_add_first('message public', 'handle_message');
