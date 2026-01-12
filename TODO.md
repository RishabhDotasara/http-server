- [x] Implement Methods 
- [x] Implement CORS
- [x] Abstract the route into a function that takes both route and method and the callback function.
- [x] Implement middlewares 

## Core Features
- [ ] Request body parsing (JSON, URL-encoded, multipart/form-data)
- [x] Query parameters parsing (/api/users?id=123)
- [ ] URL path parameters (/users/:id)
- [x] Session/Cookie support (Parse Cookie header, Set-Cookie response)
- [x] Proper error handling (500 pages, timeouts, malformed requests)
- [x] Request body size limits

## HTTP/1.1 Features
- [ ] Keep-Alive connections (connection pooling)
- [ ] Chunked transfer encoding
- [ ] Range requests (for video streaming, resume downloads)
- [ ] ETag/Last-Modified caching
- [ ] 100-Continue responses

## Security
- [ ] Security headers (X-Content-Type-Options, X-Frame-Options, CSP, HSTS)
- [ ] Rate limiting (per-IP, per-route)
- [ ] Input validation and sanitization

## Logging & Monitoring
- [ ] Access logs (combined/common format)
- [ ] Error logs
- [ ] Log rotation
- [ ] Request metrics (count, response times, error rates)
- [ ] Active connection tracking

## Static File Improvements
- [ ] Gzip/Brotli compression
- [ ] Directory listing (optional)
- [ ] Better MIME type detection

## Routing Enhancements
- [ ] Route groups/prefixes
- [ ] Route parameters (/users/:id)
- [ ] Wildcard routes (/files/*)
- [ ] Route priority/ordering

## Configuration & Deployment
- [ ] Config file support (JSON/YAML)
- [ ] Environment variables
- [ ] Graceful shutdown (handle SIGTERM/SIGINT)
- [ ] Hot reload

## Advanced Features
- [ ] WebSocket support
- [ ] Virtual hosts (multiple domains on same port)
- [ ] Reverse proxy capabilities
- [ ] HTTP/2 support
- [ ] Zero-copy file serving (sendfile()) 
