#ifndef __HELPER__
#define __HELPER__

struct URL
{
    String protocol = "";
    String host = "";
    int port = 80;
    String path = "";
} url;

const char html_header[] PROGMEM = R"===(
<!DOCTYPE HTML><html><head>
<link rel="stylesheet" type="text/css" href="radio.css">
<title>Internet Radio</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head><body>)===";

const char html_footer[] PROGMEM = R"===(
</body></html>)===";

String EncodeUrl(String url)
{
    url.replace(" ", "%20");
    url.replace("&", "%26");
    url.replace("?", "%3F");
    url.replace("=", "%3D");
    url.replace("+", "%2B");
    url.replace("#", "%23");
    url.replace("/", "%2F");
    url.replace("\\", "%5C");
    url.replace("\"", "%22");
    url.replace("<", "%3C");
    url.replace(">", "%3E");
    url.replace("|", "%7C");
    return url;
}

void parseURL(String urlString, URL *url)
{
    // Assume a valid URL

    enum URLParseState
    {
        PROTOCOL,
        SEPERATOR,
        HOST,
        PORT,
        PATH
    } state = PROTOCOL;

    url->protocol = "";
    url->host = "";
    url->path = "/";
    String port = "";

    for (int i = 0; i < urlString.length(); i++)
    {
        switch (state)
        {
        case PROTOCOL:
            if (urlString[i] == ':')
                state = SEPERATOR;
            else
                url->protocol += urlString[i];
            break;
        case SEPERATOR:
            if (urlString[i] != '/')
            {
                state = HOST;
                url->host += urlString[i];
            }
            break;
        case HOST:
            if (urlString[i] == ':')
                state = PORT;
            else
            {
                if (urlString[i] == '/')
                    state = PATH;
                else
                    url->host += urlString[i];
            }
            break;
        case PORT:
            if (urlString[i] == '/')
                state = PATH;
            else
                port += urlString[i];
            break;
        case PATH:
            url->path += urlString[i];
        }
    }

    url->port = (port.length() == 0) ? 80 : port.toInt();
}

int Get4BarsFromRSSI(int32_t rssi)
{
    if (rssi < -90) return 1;
    if (rssi < -70) return 2;
    if (rssi < -55) return 3;
    return 4;
}

#endif //__HELPER_H__