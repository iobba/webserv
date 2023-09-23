#include <iostream>
#include <curl/curl.h>

int main() {
    // Initialize libcurl
    CURL *curl = curl_easy_init();

    if (curl) {
        // Set the target URL
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3490/chhh");

        // Set the request method
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        // Set the "Transfer-Encoding: chunked" header
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the request body in chunks
        const char *chunks[] = {
            "This is the first chunk\r\n",
            "And this is the second chunk\r\n",
            "Last chunk\r\n",
        };

        for (const char *chunk : chunks) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, chunk);
            CURLcode res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "Failed to send chunk: " << curl_easy_strerror(res) << std::endl;
                break;
            }
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }

    return 0;
}


