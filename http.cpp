#include "http.h"


Http::Http()
{
    memset(&request, 0, sizeof(Request));
    
    buf_size = 8192;
    buf_data = new char[buf_size];
    
    already_read_len = 0;
    
    getcwd(webroot, sizeof(webroot));
    
    InitMimeType();
    InitStatusCode();
}

Http::~Http()
{
    if (buf_data != NULL)
        delete [] buf_data;
}

void Http::InitStatusCode()
{
    static StatusCode code[] = {
        {200, "OK"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {0, NULL}
    };
    
    codes = code;
}

void Http::InitMimeType()
{
    static MimeType type[] = {
        {".html",       "text/html"},
        {".htm",        "text/html"},
        {".shtm",		"text/html"},
        {".shtml",		"text/html"},
        {".css",		"text/css"},
        {".js",			"application/x-javascript"},
        {".ico",		"image/x-icon"},
        {".gif",		"image/gif"},
        {".jpg",		"image/jpeg"},
        {".jpeg",		"image/jpeg"},
        {".png",		"image/png"},
        {".svg",		"image/svg+xml"},
        {".torrent",	"application/x-bittorrent"},
        {".wav",		"audio/x-wav"},
        {".mp3",		"audio/x-mp3"},
        {".mid",		"audio/mid"},
        {".m3u",		"audio/x-mpegurl"},
        {".ram",		"audio/x-pn-realaudio"},
        {".ra",			"audio/x-pn-realaudio"},
        {".doc",		"application/msword"},
        {".exe",		"application/octet-stream"},
        {".zip",		"application/x-zip-compressed"},
        {".xls",		"application/excel"},
        {".tgz",		"application/x-tar-gz"},
        {".tar",		"application/x-tar"},
        {".gz",			"application/x-gunzip"},
        {".arj",		"application/x-arj-compressed"},
        {".rar",		"application/x-arj-compressed"},
        {".rtf",		"application/rtf"},
        {".pdf",		"application/pdf"},
        {".swf",		"application/x-shockwave-flash"},
        {".mpg",		"video/mpeg"},
        {".mpeg",		"video/mpeg"},
        {".asf",		"video/x-ms-asf"},
        {".avi",		"video/x-msvideo"},
        {".bmp",		"image/bmp"},
        {NULL,			NULL}};
    
    types = type;
  
}


int Http::Read(char *buf, int len)
{
    return read(client_c->fd, buf, len);
}

int Http::Write()
{
}


void Http::Read_Request()
{
    int read_len = 0;
    while (already_read_len < buf_size)
    {
        read_len = Read(buf_data + already_read_len, buf_size - already_read_len);
        if (read_len <= 0)
            break;
        else
        {
            already_read_len += read_len;
            header_len = GetHeaderLen(buf_data, already_read_len);
            if (header_len > 0)
                break;
        }
    }
    //buf_data[already_read_len] = '\0';
}

int Http::GetHeaderLen(char *buf, int len)
{
    char *begin = buf;
    char *end   = buf + len;
    
    for (; begin < end; begin++)
    {
        if(*begin == '\n' && begin < (end - 2) &&
           *(begin+1) == '\r' && *(begin+2) == '\n')
        {
           *(begin+1) = '\0';
           *(begin+2) = '\0';
           return begin - buf + 3;
        }
    }
    return 0;
}

void Http::Parse_Header()
{
    char *buf = buf_data;
    request.request_method = Skip(&buf, " ");
    request.request_uri    = Skip(&buf, " ");
    request.request_version= Skip(&buf, "\r\n");
    
    for ( ; *buf != '\0'; )
    {
        request.http_headers[request.num_headers].name = Skip(&buf, ":");
        request.http_headers[request.num_headers].value = Skip(&buf, "\r\n");
        
        request.num_headers++;
    }
}


char *Http::Skip(char **buf, const char *delimiters)
{
    char *p, *begin_word, *end_word;
    int  delimiter_len = 0;

	begin_word = *buf;
	end_word = begin_word + strcspn(begin_word, delimiters);
	
    delimiter_len = strlen(delimiters);
    for (int i = 0; i < delimiter_len; i++)
    {
        *(end_word+i) = '\0';
    }
    
    *buf = end_word + delimiter_len;

    return begin_word;
}


bool Http::CheckAuthorization()
{
    return true;
}

bool Http::HaveIndexFile(char *path, struct stat *st)
{
    strcat(path, "website/index.html");
    stat(path, st);
    return true;
}

void Http::Analyse_Request()
{
    /* get queury string  */
    request.query_string = strchr(request.request_uri, '?');
    if (request.query_string != NULL)
        *(request.query_string++) = '\0';
        
    
    char path[256];
    ConvertUriToFileName(path, sizeof(path));
    
    
    struct stat st;
    if (!CheckAuthorization())
    {
        //无权访问 401
    }
    else if (stat(path, &st) != 0)
    {
        //404 not found
        printf("404\n");
    }
    else if (S_ISDIR(st.st_mode) && request.request_uri[strlen(request.request_uri) - 1] != '/')
    {
        // 301 moved
        printf("301\n");
    }
    else if (S_ISDIR(st.st_mode) && HaveIndexFile(path, &st) == false)
    {
        //read dir
        printf("dir\n");
    }
    else if (MatchCgi(path))
    {
        if (strcmp(request.request_method, "GET") && strcmp(request.request_method, "POST"))
            ;//ERROR
        else
            SendCgi(path);
    }
    else
    {
        SendFile(path, &st);
    }
}

void Http::AddEnv(char *env, const char *format, const char *value)
{
    char ft[100];
    strcpy(ft, "%s");
    strcat(ft, format);
    
    sprintf(env, ft, env, value);
}

void Http::SendCgi(char *path)
{
    char env[8192];
    BuildCgiEnv(path, env);
    
    printf("%s\n", env);
}

void Http::BuildCgiEnv(char *path, char *env)
{
    AddEnv(env, "%s", "GATEWAY_INTERFACE=CGI/1.1");
    AddEnv(env, "%s", "SERVER_PROTOCOL=HTTP/1.1");
    AddEnv(env, "%s", "REDIRECT_STATUS=200");
    AddEnv(env, "REQUEST_METHOD=%s", request.request_method);
    AddEnv(env, "PATH=%s", "tmp_paht");
}

const char *Http::GetStatusReason(int status)
{
    int i = 0;
    while (codes[i].code != 0)
    {
        if(codes[i].code == status)
            return codes[i].text;
        i++;
    }
    
    return "ERROR";
}


void Http::SendHeader(const char *mime_type, int file_size, int status_code)
{
    char buf[8192];
    memset(buf, 0, sizeof(buf));
    
    sprintf(buf, "HTTP/1.0 %d %s\r\n", status_code, GetStatusReason(status_code));
	sprintf(buf, "%sServer: Spch2008 Web Server\r\n", buf);
	sprintf(buf, "%sContent-length:%d\r\n", buf, file_size);
	sprintf(buf, "%sContent-type:%s\r\n\r\n", buf, mime_type);
    
    write(client_c->fd, buf, strlen(buf));
}

const char *Http::GetMimeType(char* postfix)
{
    int i = 0;
    while (types[i].extension != NULL)
    {
        if (strcmp(types[i].extension, postfix) == 0)
            return types[i].mime_type;
        i++;
    }
    
    return "text/plain";
}

void  Http::SendFile(char *path, struct stat *st)
{
    const char *mime_type = GetMimeType(strrchr(path, '.'));
    
    SendHeader(mime_type, st->st_size, 200);
    
    if (strcmp(request.request_method, "HEAD") != 0)
        SendFileStream(path, st);
}

void Http::SendFileStream(char *path, struct stat *st)
{

    int srcfd;
	void *srcp;

    srcfd = open(path, O_RDONLY, 0);
	srcp  = mmap(0, st->st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	write(client_c->fd, (char*)srcp, st->st_size);
	munmap(srcp, st->st_size);
}

void Http::ConvertUriToFileName(char *path, int len)
{
    char *uri = request.request_uri;
    
    strcpy(path, webroot);
    strcat(path, uri);
}

bool Http::MatchCgi(char *path)
{
    const char *target = ".php; .cgi; .py";
    const char *suffix = strrchr(path, '.');
    
    if (strstr(target, suffix) != NULL)
        return true;
    else
        return false;
    
}

void Http::CloseConnection()
{
    close(client_c->fd);
}

void Http::HandleRequest(Connection *c)
{
    client_c = c;
    
    Read_Request();
    Parse_Header();
    Analyse_Request();
    
    CloseConnection();
}