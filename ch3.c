///
/// Chapter 2: Networking
/// =====================
///
/// In this chapter we will begin to investigate what it will take to write the networking code that
/// will implement HTTP 1.1 in as little code as possible.
///
/// HTTP/1.1
/// --------
///
/// Just what is the HTTP/1.1 thing anyway? HTTP is a fairly simple networking protocol. We will be
/// simplifying it even further as we shrink the code complexity down to as simple as possible. At
/// its most basic, HTTP has a request format and a response format. Here is what they look like:
///
/// ### HTTP Request
///     GET /index.html HTTP/1.1
///     Host: www.google.com
///
/// ### HTTP Response
///     HTTP/1.1 200 OK
///     Content-length: 331
///     
///     <html>...</html>
///
/// As you can see, there isn't that much to it. This is ignore a lot of the special case scenarios you
/// may encounter, like url redirects, but those will come in time. The important things to note are that
/// the messages are simple. All lines end in Windows-style line endings (`CRLF` or `\r\n`). The end of the
/// message is indicated by two end-of-line markers in a row (`CRLFCRLF` or `\r\n\r\n`). If we can send and
/// receive these messages, we should be most of the way there.
///
/// Includes
/// --------
///
/// Here are the headers that we used in Chapter 1:
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// In addition to string handling and I/O, we are going to use the standard networking headers as well as
/// regular expression support to perform some more complicated processing:
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <regex.h>

///
/// Communicating with a Server
/// ---------------------------
///
/// Define a simple macro to handle defining a static regex. This will be used a number of times over,
/// so defining it here will save some space. The macro makes use of the single field that is required to be
/// defined inside of a `regex_t` structure, `.re_nsub`. By setting it to -1 in the static initialization,
/// we can check for it equally -1 later to only run our compilation code once. In a more complicated
/// environment with threading we would need to be more careful, but this should be fine for our simple
/// example.
#define REGEX(name, string) \
    static regex_t name = { .re_nsub=-1 }; \
    if (name.re_nsub == -1) regcomp(&name, string, REG_EXTENDED | REG_ICASE); 

/// Define the maximum size of a response from a server. Setting this to be a fixed constant will simplify the code,
/// but make it less flexible. A more flexible solution might use realloc to adjust the size of the response buffer.
#define MAX_RESPONSE_SIZE (1<<17)

/// Our new function, `navigate_to_url()`, defines what should be done with a new URL. For now, it will
/// use a regular expression to parse the URL, open a socket with the results, and send and receive a very
/// simple HTTP message.
void navigate_to_url(char *url) {
    REGEX(url_regex, "(http://)?([^/:]+)(:([0-9]+))?(/(.*))?");
    regmatch_t matches[32];
    if (regexec(&url_regex, url, url_regex.re_nsub + 1, matches, 0) == 0) { // the third argument of regexec takes the number of submatches, not the total number of matches.
        // 2 is host, 4 is port, 6 is path without the initial slash
        char *host = "";
        if (matches[2].rm_so != -1) { // a match succeeded if it is not -1
            host = url + matches[2].rm_so; // a match begins at `.rm_so`,
            url[matches[2].rm_eo] = '\0';  // and should be terminated at `.rm_eo`. I have constructed the regex to ensure that the next character is never important.
        }
        char *port = "80";
        if (matches[4].rm_so != -1) {
            port = url + matches[4].rm_so;
            url[matches[4].rm_eo] = '\0';
        }
        char *path = "";
        if (matches[6].rm_so != -1) {
            path = url + matches[6].rm_so;
            url[matches[6].rm_eo] = '\0';
        }

        struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
        struct addrinfo *serv = NULL;
        if (getaddrinfo(host, port, &hints, &serv) == 0) {
            int sock = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
            if (sock > -1) {
                if (connect(sock, serv->ai_addr, serv->ai_addrlen) > -1) {
                    // send a message to the remote server
                    char *message; asprintf(&message, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, host);
                    send(sock, message, strlen(message), 0);
                    free(message);

                    // receive all of the incoming message
                    char *response = malloc(MAX_RESPONSE_SIZE);
                    char *received = response;
                    int bytes_received = 0;
                    while ((bytes_received = recv(sock, received, response + MAX_RESPONSE_SIZE - received - 1, 0)) > 0) {
                        received += bytes_received;
                    }
                    *received = '\0';
                    printf("Response from %s:%s (/%s):\n%s\n", host, port, path, response);
                    free(response);
                }
            }
        }
    }
}

/// Here is what this function is doing:
///
/// * First, a regular expression is defined to parse URLs. It is a very permissive regular expression, but should get
///   the job done. Here is the breakdown of what is in the regex:
///   * `(http://)?`: Optionally allow the string to begin with `http://`.
///   * `([^/:]+)`: Define the hostname of the URL to be anything after the `http://` that does contain a colon or a slash.
///   * `(:([0-9]+))?`: Optionally grab a port number, and place just the number portion without the colon in a group.
///   * `(/(.*))?`: Optionally grab a path following the hostname and port. Notice that the beginning slash is not part of
///     the definition.
///   This regex places the matches into groups. Hostname is group 2, port is group 4, and the path is group 6.
///
/// * Next, pull the strings out of the matches, null terminating along the way. We use the fact that the beginning slash is
///   not part of the path name to allow us to have a null terminator between the host/port and path.
///
/// * Once we have the full request information parsed, we construct a socket, connect to the server, and send a simple request
///   in the form of an HTTP GET.
///
/// * We then wait for a response, pulling the response in pieces into a larger response buffer. Again, we make sure we NULL
///   terminate everything.
///
/// This leaves us well prepared for our next chapter, where we will parse the response data with a new set of regular
/// expressions to derive a very minimal ASCII representation of the page.
///
/// And finally, here is the original main function modified to use our new function when a URL is given as input:
int main(int argc, char **argv) {
    while (1) {
        printf("? ");
        char inbuf[512];
        fgets(inbuf, sizeof(inbuf), stdin);
        int link_num = atoi(inbuf);
        if (link_num) {
            printf("navigate to the link %d\n", link_num);
        } else {
            navigate_to_url(inbuf);
        }
    }
    return 0;
}
