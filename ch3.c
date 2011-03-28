///
/// Chapter 3: "Parsing" HTML
/// =========================
///
/// In this chapter we will create a fully armed and operational web browser, though it will be
/// missing some very important features.
///
/// HTML5
/// -----
///
/// This web browser will be a fully functional HTML5 client. I do not mean we will achieve 100% on the
/// acid 3 test in 128 lines of code, but rather we will embrace the idea that HTML is a fluid language
/// that should gracefully degrade on less featureful clients. Our support of HTML will put this to the
/// test.
///
/// The plan is as follows:
///
/// * Get the full HTML text (by stripping out the HTTP headers that we looked at in the previous chapter.
///
/// * All valid HTML documents that we display should have the following form:
///       <html>
///           ...
///           <body>
///               [The stuff we care about]
///           </body>
///       </html>
///   Thus, by looking for the `<body>` and `</body>` tags, we should be able to find all of the content we
///   need.
///
/// * Once we have the code inside the body, we will do a very minimal formatting job:
///   * For every `<a href="">` tag we encounter, we will will make a note of the URL in the `href`, and add
///     a corresponding number to the outputted text. The user will be able to use this number to follow the
///     link. How quaint!
///   * For every `<br>` tag we encounter, we will add a newline to make the text a little prettier.
///
/// Includes
/// --------
///
/// Here are the headers we have used up to now. We will not add anything more in this chapter, instead
/// relying on the tools we already have available to us.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <regex.h>

/// Keep the useful macros up at the top:
#define REGEX(name, string) \
    static regex_t name = { .re_nsub=-1 }; \
    if (name.re_nsub == -1) regcomp(&name, string, REG_EXTENDED | REG_ICASE); 
#define MAX_RESPONSE_SIZE (1<<17)

///
/// Handling specific HTTP responses
/// --------------------------------
///
/// An HTTP response can take a couple forms. It can be successful (codes 2XX), a redirection (codes 3XX),
/// or an error (codes 4XX for client errors, code 5XX for server errors). Each of these individual functions
/// will handle these scenarios. The next section will cover how the information for each of these functions
/// is parsed from the response.
void handle_http_success(int code, char *header, char *message) {
    // Assuming that we have HTML, go handle the HTML!
    printf("Success: %d\n\n%s\n\n%s\n", code, header, message);
}

void handle_http_redirect(int code, char *header, char *message) {
    // look for the line "Location: new-url\r\n" in the header
    printf("Redirect: %d\n\n%s\n\n%s\n", code, header, message);
}

void handle_http_error(int code, char *error, char *header, char *message) {
    printf("Error: %d (%s)\n\n%s\n\n%s\n", code, error, header, message);
}

///
/// Handling the HTTP response
/// --------------------------
///
/// This method will handle all HTTP responses. For now, we will expect the response to be a 200 OK,
/// but in the future we will need to handle other response types to allow for a more robust web
/// browser.
void handle_http_response(char *response) {
    REGEX(http_response_regex, "(.*)\r?\n\r?\n(.*)");
    regmatch_t matches[32];
    if (regexec(&http_response_regex, response, http_response_regex.re_nsub + 1, matches, 0) == 0) {
        char *header = NULL;
        if (matches[1].rm_so != -1) {
            header = response + matches[1].rm_so;
            response[matches[1].rm_eo] = '\0';
        }
        char *message = "";
        if (matches[2].rm_so != -1) {
            message = response + matches[2].rm_so;
            response[matches[2].rm_eo] = '\0';
        }
        
        printf("header: %s\n\nmessage: %s\n", header, message);

        REGEX(http_response_status_line_regex, "[^\\s]+ ([\\d]+) (.*)\r?\n");
        if (regexec(&http_response_status_line_regex, header, http_response_status_line_regex.re_nsub + 1, matches, 0) == 0) {
            int code = 0;
            if (matches[1].rm_so != -1) {
                code = atoi(header + matches[1].rm_so);
            }
            char *error = "";
            if (matches[2].rm_so != -1) {
                error = response + matches[2].rm_so;
                header[matches[2].rm_eo] = '\0';
                header = header + matches[2].rm_eo + 1;
            }

            if (code >= 200 && code < 300) {
                handle_http_success(code, header, message);
            } else if (code < 400) {
                handle_http_redirect(code, header, message);
            } else if (code < 600) {
                handle_http_error(code, error, header, message);
            }
        }
    }
}

/// And finally, here is our original code, modified to call our html parsing code
void navigate_to_url(char *url) {
    REGEX(url_regex, "(http://)?([^/:[:space:]]+)(:([0-9]+))?(/([^[:space:]]*))?");
    regmatch_t matches[32];
    if (regexec(&url_regex, url, url_regex.re_nsub + 1, matches, 0) == 0) {
        // 2 is host, 4 is port, 6 is path without the initial slash
        char *host = "";
        if (matches[2].rm_so != -1) {
            host = url + matches[2].rm_so;
            url[matches[2].rm_eo] = '\0';
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
                    char *message; asprintf(&message, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
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
                    handle_http_response(response);
                    free(response);
                }
            }
        }
    }
}

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
