#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http.h"

#define BUF_SIZE 1024
#define REQ_FORMAT_LEN 56
#define REQ_FORMAT "%s /%s%s HTTP/1.0\r\nHost: %s\r\n%sUser-Agent: getter\r\n\r\n"
#define RANGE_FORMAT "Range: bytes=%s\r\n"

/**
 * @brief Get the range object
 *         tab.act
 * @param range 
 * @return char* 
 */

char *range_field_format(const char *range)
{
    char *range_query = malloc(strlen(RANGE_FORMAT) + strlen(range));
    sprintf(range_query, RANGE_FORMAT, range);
    return range_query;
}

// /**
//  * @brief Generates a request string for either HEAD or a GET request
//  * Embeds a head or get request with the range specified.
//  * @param page URL to request <page?
//  * @param host URL to request <host>
//  * @param get    Is get request (0 = No, !0 = Yes)
//  * @param range  Speciify range. If range is not set; do not use range.
//  * @return char* the query string.
//  */
char *query_string_formatter(char *page, char *host, int get, const char *range)
{
    char *req = (get) ? "GET" : "HEAD";
    char *blank = "";
    char *query = malloc(strlen(host) + strlen(page) + 1 + strlen(req) + strlen(range) + REQ_FORMAT_LEN + 3);
    if (page[0 == '/'])
    {
        sprintf(query, REQ_FORMAT, req, "", page, host, (range != blank) ? range_field_format(range) : "");
    }
    else
    {
        sprintf(query, REQ_FORMAT, req, "/", page, host, (range != blank) ? range_field_format(range) : "");
    }
    return query;
}

/*Free buffer
*/
void buff_free(Buffer* buffer) { 
    free(buffer->data);
    free(buffer);
}
/**
 * @Connects a TCP socket on hostname, portnum,
 * on fail, exit 
 * returns a handle for this
 * 
 * @param hostname 
 * @param portnum 
 * @return int 
 */
int make_socket(char *hostname, int portnum)
{
    
    int sockfd;
    int rc; // for return codes
    struct addrinfo their_addrinfo;
    struct addrinfo *their_addr = NULL;
    char port[6];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //Error grabbing a socket: terminate.
    if (sockfd == -1)
    {
        perror("Creating Socket\n");
        exit(1);
    }
    memset(&their_addrinfo, 0, sizeof(struct addrinfo));
    their_addrinfo.ai_family = AF_INET;
    their_addrinfo.ai_socktype = SOCK_STREAM;

    sprintf(port, "%d", portnum);
    getaddrinfo(hostname, port, &their_addrinfo, &their_addr);

    rc = connect(sockfd, their_addr->ai_addr, their_addr->ai_addrlen);
    freeaddrinfo(their_addr);
    if (rc == -1)
    {
        perror("Connecting To Server\n");
        close(sockfd);
        exit(1);
    }
    return sockfd;
}

/**
 * Generate a Buffer
 * @param size - The length of data contained within the buffer
 * @return Buffer* 
 */
Buffer *make_buffer(size_t size)
{
    Buffer *make = malloc(sizeof(Buffer));
    make->data = (char *)calloc(size, sizeof(char));
    make->length = size;
    return make;
}

/**
 * Place received data into a buffer; buffer length is set at consant, conveniently the same as above.
 * @param buff 
 * @param sockfd 
 */
void recv_data(Buffer *buff, int sockfd)
{
 
    int rec, total = 0;
    do
    {
        rec = recv(sockfd, buff->data + total, BUF_SIZE, 0);
        if (rec == -1)
        {
            close(sockfd);
            //free the buffer
            exit(1);
        }
        total += rec;
        //reallocate space for buffer as necessary.
        //linear increase
        if (buff->length <= total + BUF_SIZE)
        {
        
            buff->length = buff->length + BUF_SIZE;
            buff->data = realloc(buff->data, buff->length);
            if (!buff->data) { 
                perror("realloc");
                free(buff->data);
                exit(1);
            }
        
        
        }

    } while (rec > 0);
    close(sockfd);
    buff->length = total;
}


/**
 * Perform an HTTP 1.0 query to a given host and page and port number.
 * host is a hostname and page is a path on the remote server. The query
 * will attempt to retrievev content in the given byte range.
 * User is responsible for freeing the memory.
 * 
 * @param host - The host name e.g. www.canterbury.ac.nz
 * @param page - e.g. /index.html
 * @param range - Byte range e.g. 0-500. NOTE: A server may not respect this
 * @param port - e.g. 80
 * @return Buffer - Pointer to a buffer holding response data from query
 *                  NULL is returned on failure.
 */
Buffer *http_query(char *host, char *page, const char *range, int port)
{
    // Connect w/ TCP for http request
    Buffer *rec = make_buffer(BUF_SIZE);
    int sockfd = make_socket(host, port);
    char *req = query_string_formatter(page, host, 1, range);
    int rc = send(sockfd, req, strlen(req), 0);
    if (rc == -1)
    {
        perror("Send Query");
        free(req);
        close(sockfd);
        exit(1);
    }
    recv_data(rec, sockfd);
    free(req);
    close(sockfd);
    return rec;
}

/**
 * Separate the content from the header of an http request.
 * NOTE: returned string is an offset into the response, so
 * should not be freed by the user. Do not copy the data.
 * @param response - Buffer containing the HTTP response to separate 
 *                   content from
 * @return string response or NULL on failure (buffer is not HTTP response)
 */
char *http_get_content(Buffer *response)
{

    char *header_end = strstr(response->data, "\r\n\r\n");

    if (header_end)
    {
        return header_end + 4;
    }
    else
    {
        return response->data;
    }
}

/**
 * Splits an HTTP url into host, page. On success, calls http_query==
 * to execute the query against the url. 
 * @param url - Webpage url e.g. learn.canterbury.ac.nz/profile
 * @param range - The desired byte range of data to retrieve from the page
 * @return Buffer pointer holding raw string data or NULL on failure
 */
Buffer *http_url(const char *url, const char *range)
{
    char host[BUF_SIZE];
    strncpy(host, url, BUF_SIZE);
    char *page = strstr(host, "/");
    if (page)
    {
        page[0] = '\0';
        ++page;
        return http_query(host, page, range, 80);
    }
    else
    {
        fprintf(stderr, "could not split url into host/page %s\n", url);
        return NULL;
    }
}

/**
 * @brief Extracts the specified field from a response string
 * This IS transformative, but we dont need the header after 
 * this step
 * @param Header - data to parse
 * @return parsed val
 */
int extract_content_size(Buffer *header)
{
    int scale = 1;
    int ret = -1;
    char *line;
    char accept[10];
    line = strtok(header->data, "\n");
    while (line != NULL)
    {
        if (strstr(line, "Content-Length:") != NULL)
        {
            sscanf(line, "Content-Length: %d", &ret);
        }
        else if (strstr(line, "Accept-Ranges:") != NULL)
        {
            sscanf(line, "Accept-Ranges:%s", accept);
            if (strcmp(accept, "bytes") != 0)
                scale = -1;
        }

        line = strtok(NULL, "\n");
    }
    free(line);
    return ret * scale;
}

/**
 * Makes a HEAD request to a given URL and gets the content length
 * Then determines max_chunk_size and number of split downloads needed
 * @param url   The URL of the resource to download
 * @param threads   The number of threads to be used for the download
 * @return int  The number of downloads needed satisfying max_chunk_size
 *              to download the resource
 */
int get_num_tasks(char *url, int threads)
{
    Buffer *rec_data = make_buffer(BUF_SIZE);
    int sockfd = 0;
    int content_size = 0;
    char host[BUF_SIZE];
    strncpy(host, url, BUF_SIZE);
    char *page = strstr(host, "/");
    if (page)
    {
        page[0] = '\0';
        ++page;
        sockfd = make_socket(host, 80);
    }
    else
    {
        perror("split URL in head request");
        exit(0);
    }
    //HEAD request
    char *req = query_string_formatter(page, host, 0, "");
    int rc = send(sockfd, req, strlen(req), 0);
    if (rc == -1)
    {
        perror("Send Query");
        free(req);
        close(sockfd);
        exit(1);
    }
    recv_data(rec_data, sockfd);
    //calculatre max_chunk_size, num_downloads
    content_size = extract_content_size(rec_data);
    if (content_size < 0)
    {
        printf("* The server reports that it does not support range specification");
        max_chunk_size = content_size * -1;
        return 1;
    }
    max_chunk_size = (content_size / (threads));
    close(sockfd);
    free(req);
    buff_free(rec_data);
    return threads;
}

int get_max_chunk_size()
{
    return max_chunk_size;
}
