///
/// Chapter 1: The beginning
/// ========================
///
/// In this chapter we will lay the ground rules for what is being attempted, and begin a simple
/// foundation which will be built upon in the subsequent chapters.
///
/// Rules
/// -----
///
/// The executable we are creating needs to be able to open a socket, send and receive text, and
/// process this text. It needs to understand enough of what it is processing to at least let the
/// user advance to another page. Based on this analysis, I believe that the minimal web browser
/// needs to support a very minimal implementation of HTTP/1.0 and needs to understand only the
/// `<a>` tag in HTML.
///
/// Includes
/// --------
///
/// The application makes use of as much functionality as can be pulled from POSIX, which will include
/// the networking, string manipulation, and regex code. Other than that, everything will be written
/// from scratch.
#include <stdio.h>
#include <string.h>

/// The main runloop
/// ----------------
///
/// The application will loop on input until the user quits by typing ctrl-c. Otherwise, all input is
/// interpreted as a request to follow a numbered link, or a URL.

int main(int argc, char **argv) {
  while (1) {
    printf("? ");
    char inbuf[512];
    fgets(inbuf, sizeof(inbuf), stdin);
    int link_num = atoi(inbuf);
    if (link_num) {
      printf("navigate to the link %d\n", link_num);
    } else {
      printf("navigate to the url %s\n", inbuf);
    }
  }
  return 0;
}
