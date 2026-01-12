#pragma once 
#include "server.hpp"

// --- URL Decoder Middleware 
void urlDecode(Request& req, Response& res, Next next);

// Query Parameter Exrtractor 
void paramExtractor(Request& req, Response& res, Next next);

// JSON Body Parser 
void parseJson(Request& req, Response& res, Next next);

// Rate Limiter Middleware
void rateLimit(Request& req, Response& res, Next next);