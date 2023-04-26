#ifndef BYTEDANCE_AILAB_CV_LICENSE_REQUESTER_H_
#define BYTEDANCE_AILAB_CV_LICENSE_REQUESTER_H_

int request_license(
    const char* key,
    const char* secret,
    const char* authMsg,
    char** license,
    int* size);


void get_request_content(
    const char* key,
    const char* secret,
    const char* authMsg,
    char** content,
    int* size
);

void free_content(char* content);

#endif