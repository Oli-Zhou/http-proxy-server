# erss_hw2_http_proxy_hb174_az161

1. when the http response is too long, proxy may response slow because of long body string processing.

2. Robust to external failure:
    - Server accepts socket connection termination by throwing exception and deal with socket disconnection
    - domain name resolve failure can be checked and prevent
    - I/O selection fail checking
    - file descriptor failure check
3. Exception handle 
    - Socket initilize exception
    - I.O selection fail exception
    - Domain name resolves to ip exception.
