#ifndef _HTTP_H_
#define _HTTP_H_

#include "core.h"

class Http
{
public:
    Http();
    ~Http();
    
    void HandleRequest(Connection *c);
    
protected:

    void InitMimeType();
    void InitStatusCode();

    int Read(char *buf, int len);
    int Write();
    int GetHeaderLen(char *buf, int len);
    
    void Read_Request();
    void Parse_Header();
    char *Skip(char **buf, const char *delimiter);
    
    void Analyse_Request();
    void ConvertUriToFileName(char *path, int len);
    bool IsKnownMethod();
    bool CheckAuthorization();
    bool HaveIndexFile(char *path, struct stat *st);
    
    bool MatchCgi(char *path);
    void BuildCgiEnv(char *path, char *env);
    void AddEnv(char *env, const char *format, const char *value);
    
    const char *GetMimeType(char* postfix);
    const char *GetStatusReason(int status);
    void  SendFile(char *path, struct stat *st);
    void  SendFileStream(char *path, struct stat *st);
    
    void  SendHeader(const char *mime_type, int file_size, int status_code);
    
    
    void PrepareCgiEnvironment();
    void SendCgi(char *path);
    
    void CloseConnection();
    
private:
    char *buf_data;
    int   buf_size;
    
    Request request;
    Connection *client_c;
    
    /* head len */
    int already_read_len;
    int header_len;
    
    /* root path */
    char webroot[256];
    
    /* mine type */
    MimeType *types;
    
    /*status code */
    StatusCode *codes;
};














#endif