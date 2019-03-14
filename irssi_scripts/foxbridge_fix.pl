use strict;
use vars qw($VERSION %IRSSI);

use Irssi;
$VERSION= '0.00.01';
%IRSSI = (
  authors => 'Gynvael Coldwind',
  contact => 'gynvael@coldwind.pl',
  name => 'foxbridge_fix',
  description => 'Discord bridge utilities, e.g. displaying messages in a better way.',
  license => 'BSD style',
  url => 'https://gynvael.coldwind.pl/'
);

sub handle_message {
  my ($server, $msg, $nick, $nick_addr, $channel, @x) = @_;
  return unless $nick eq 'foxbridge';
  &event_message;
}

sub event_message {
  my ($server, $msg, $nick, @rest) = @_;
  $nick = $msg;
  $nick =~ s/>.*$//;
  $nick =~ s/<//;
  $nick = 'D|' . $nick;
  $msg =~ s/^.*?> //;
  Irssi::signal_continue($server, $msg, $nick, @rest);
}

Irssi::signal_add_first('message public', 'handle_message');


