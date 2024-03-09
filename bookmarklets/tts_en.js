window.slang='en-EN'; /* change this to whatever language you need */
function processText(text) {
  let s = text.replace(/\n/g, " ").replace(/\t/g, " ");

  while (s.includes("  ")) {
    s = s.replace(/  /g, " ");
  }

  return createSentences(s.split(" "));
}

function createSentences(words) {
  let sentences = [];
  let currentSentence = "";

  while (words.length > 0) {
    let word = words.shift();

    if (currentSentence.length + word.length + 1 > 200) {
      words.unshift(word);
      sentences.push(currentSentence.trim());
      currentSentence = "";
    } else {
      currentSentence += " " + word;
      if (currentSentence.length > 50 && word.endsWith(".")) {
        sentences.push(currentSentence.trim());
        currentSentence = "";
      }
    }
  }

  if (currentSentence.trim().length > 0) {
    sentences.push(currentSentence.trim());
  }

  return sentences;
}

function speakNext() {
  if (window.sentences.length == 0) {
    console.log('The End.');
    return;
  }

  const s = window.sentences.shift();

  var utterance = new SpeechSynthesisUtterance(s);
  utterance.lang = window.slang;
  console.log('Speaking:', s)
  utterance.onend = function(event) {
    window.speechSynthesis.cancel();
    speakNext()
  };
  utterance.onerror = function(event) {
    console.error('SpeechSynthesisUtterance.onerror', event.error);
  };
  window.speechSynthesis.speak(utterance);
}

window.speechSynthesis.cancel();
window.sentences = processText(window.getSelection().toString());
speakNext();
