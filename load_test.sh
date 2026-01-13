#!/bin/bash
# filepath: /home/sinosuke/Documents/http-server/load_test.sh

# Configuration
SERVER="http://localhost:3000"
RESULTS_FILE="load_test_results.txt"
CONCURRENT_USERS=50
REQUESTS_PER_USER=20
TOTAL_REQUESTS=$((CONCURRENT_USERS * REQUESTS_PER_USER))

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Clear previous results
echo "=== Load Test Results ===" > $RESULTS_FILE
echo "Date: $(date)" >> $RESULTS_FILE
echo "Server: $SERVER" >> $RESULTS_FILE
echo "Concurrent Users: $CONCURRENT_USERS" >> $RESULTS_FILE
echo "Requests per User: $REQUESTS_PER_USER" >> $RESULTS_FILE
echo "Total Requests: $TOTAL_REQUESTS" >> $RESULTS_FILE
echo "======================================" >> $RESULTS_FILE
echo "" >> $RESULTS_FILE

# Print header
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     HTTP Server Load Test Suite       ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Test endpoints
STATIC_ENDPOINTS=(
    "/index"
    "/public/style.css"
    "/public/404.html"
)

DYNAMIC_ENDPOINTS=(
    "/users/123"
    "/users/john_doe"
    "/posts/42/comments/99"
    "/api/v1/org/acme/team/dev"
    "/shop/electronics/product/iphone"
    "/users/alice/posts/1"
    "/users/bob/posts/2"
    "/users/charlie/posts/3"
)

# Function to make a single request and measure time
make_request() {
    local url=$1
    local start=$(date +%s%N)
    local response=$(curl -s -o /dev/null -w "%{http_code}:%{time_total}" "$url" 2>&1)
    local end=$(date +%s%N)
    
    local status_code=$(echo $response | cut -d':' -f1)
    local time_total=$(echo $response | cut -d':' -f2)
    
    echo "${status_code},${time_total}"
}

# Function to test an endpoint
test_endpoint() {
    local endpoint=$1
    local name=$2
    local url="${SERVER}${endpoint}"
    
    echo -e "${YELLOW}Testing: ${name}${NC}"
    echo "Testing: ${name}" >> $RESULTS_FILE
    echo "URL: ${url}" >> $RESULTS_FILE
    
    local success_count=0
    local error_count=0
    local total_time=0
    local min_time=999999
    local max_time=0
    
    # Array to store response times for percentile calculation
    declare -a response_times
    
    # Run concurrent requests
    local pids=()
    for ((i=1; i<=CONCURRENT_USERS; i++)); do
        (
            for ((j=1; j<=REQUESTS_PER_USER; j++)); do
                result=$(make_request "$url")
                echo "$result"
            done
        ) &
        pids+=($!)
    done
    
    # Collect results from background processes
    for pid in "${pids[@]}"; do
        while IFS=',' read -r status time; do
            if [[ $status == "200" ]] || [[ $status == "304" ]]; then
                ((success_count++))
            else
                ((error_count++))
            fi
            
            # Convert time to milliseconds
            time_ms=$(echo "$time * 1000" | bc)
            total_time=$(echo "$total_time + $time_ms" | bc)
            response_times+=($time_ms)
            
            # Update min/max
            if (( $(echo "$time_ms < $min_time" | bc -l) )); then
                min_time=$time_ms
            fi
            if (( $(echo "$time_ms > $max_time" | bc -l) )); then
                max_time=$time_ms
            fi
        done < <(wait $pid; cat)
    done
    
    # Calculate statistics
    local avg_time=$(echo "scale=2; $total_time / $TOTAL_REQUESTS" | bc)
    local success_rate=$(echo "scale=2; ($success_count / $TOTAL_REQUESTS) * 100" | bc)
    local throughput=$(echo "scale=2; $TOTAL_REQUESTS / ($total_time / 1000)" | bc)
    
    # Sort response times for percentiles
    IFS=$'\n' sorted_times=($(sort -n <<<"${response_times[*]}"))
    unset IFS
    
    local p50_idx=$((TOTAL_REQUESTS * 50 / 100))
    local p95_idx=$((TOTAL_REQUESTS * 95 / 100))
    local p99_idx=$((TOTAL_REQUESTS * 99 / 100))
    
    local p50=${sorted_times[$p50_idx]:-0}
    local p95=${sorted_times[$p95_idx]:-0}
    local p99=${sorted_times[$p99_idx]:-0}
    
    # Print results
    echo -e "  ${GREEN}✓ Success: ${success_count}/${TOTAL_REQUESTS}${NC}"
    echo -e "  ${RED}✗ Errors: ${error_count}${NC}"
    echo -e "  Avg Response Time: ${avg_time}ms"
    echo -e "  Min/Max: ${min_time}ms / ${max_time}ms"
    echo -e "  P50: ${p50}ms | P95: ${p95}ms | P99: ${p99}ms"
    echo -e "  Success Rate: ${success_rate}%"
    echo -e "  Throughput: ${throughput} req/sec"
    echo ""
    
    # Write to results file
    echo "  Success: ${success_count}/${TOTAL_REQUESTS}" >> $RESULTS_FILE
    echo "  Errors: ${error_count}" >> $RESULTS_FILE
    echo "  Average Response Time: ${avg_time}ms" >> $RESULTS_FILE
    echo "  Min Response Time: ${min_time}ms" >> $RESULTS_FILE
    echo "  Max Response Time: ${max_time}ms" >> $RESULTS_FILE
    echo "  P50: ${p50}ms" >> $RESULTS_FILE
    echo "  P95: ${p95}ms" >> $RESULTS_FILE
    echo "  P99: ${p99}ms" >> $RESULTS_FILE
    echo "  Success Rate: ${success_rate}%" >> $RESULTS_FILE
    echo "  Throughput: ${throughput} req/sec" >> $RESULTS_FILE
    echo "--------------------------------------" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE
}

# Function to test keep-alive effectiveness
test_keepalive() {
    echo -e "${BLUE}Testing Keep-Alive Connection...${NC}"
    echo "Testing Keep-Alive Connection" >> $RESULTS_FILE
    
    local url="${SERVER}/index"
    local requests=10
    
    # Test with keep-alive
    local start_ka=$(date +%s%N)
    for ((i=1; i<=requests; i++)); do
        curl -s -o /dev/null "$url" --keepalive-time 60
    done
    local end_ka=$(date +%s%N)
    local time_ka=$(echo "scale=2; ($end_ka - $start_ka) / 1000000" | bc)
    
    # Test without keep-alive
    local start_no_ka=$(date +%s%N)
    for ((i=1; i<=requests; i++)); do
        curl -s -o /dev/null "$url" -H "Connection: close"
    done
    local end_no_ka=$(date +%s%N)
    local time_no_ka=$(echo "scale=2; ($end_no_ka - $start_no_ka) / 1000000" | bc)
    
    local improvement=$(echo "scale=2; (($time_no_ka - $time_ka) / $time_no_ka) * 100" | bc)
    
    echo -e "  With Keep-Alive: ${time_ka}ms"
    echo -e "  Without Keep-Alive: ${time_no_ka}ms"
    echo -e "  ${GREEN}Improvement: ${improvement}%${NC}"
    echo ""
    
    echo "  With Keep-Alive: ${time_ka}ms" >> $RESULTS_FILE
    echo "  Without Keep-Alive: ${time_no_ka}ms" >> $RESULTS_FILE
    echo "  Improvement: ${improvement}%" >> $RESULTS_FILE
    echo "--------------------------------------" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE
}

# Function to test rate limiting
test_rate_limiting() {
    echo -e "${BLUE}Testing Rate Limiting...${NC}"
    echo "Testing Rate Limiting" >> $RESULTS_FILE
    
    local url="${SERVER}/index"
    local limit_requests=10
    local rate_limited=0
    
    for ((i=1; i<=limit_requests; i++)); do
        status=$(curl -s -o /dev/null -w "%{http_code}" "$url")
        if [[ $status == "429" ]]; then
            ((rate_limited++))
        fi
    done
    
    echo -e "  Sent ${limit_requests} requests"
    echo -e "  Rate Limited (429): ${rate_limited}"
    
    if [[ $rate_limited -gt 0 ]]; then
        echo -e "  ${GREEN}✓ Rate limiting is working${NC}"
    else
        echo -e "  ${YELLOW}⚠ No rate limiting detected${NC}"
    fi
    echo ""
    
    echo "  Total Requests: ${limit_requests}" >> $RESULTS_FILE
    echo "  Rate Limited: ${rate_limited}" >> $RESULTS_FILE
    echo "--------------------------------------" >> $RESULTS_FILE
    echo "" >> $RESULTS_FILE
}

# Main test execution
echo -e "${YELLOW}Starting Load Tests...${NC}\n"

# Test static endpoints
echo -e "${BLUE}=== Static Endpoints ===${NC}"
for endpoint in "${STATIC_ENDPOINTS[@]}"; do
    test_endpoint "$endpoint" "Static: $endpoint"
done

# Test dynamic endpoints (route parameter extraction)
echo -e "${BLUE}=== Dynamic Endpoints (Route Parameters) ===${NC}"
for endpoint in "${DYNAMIC_ENDPOINTS[@]}"; do
    test_endpoint "$endpoint" "Dynamic: $endpoint"
done

# Test keep-alive
test_keepalive

# Test rate limiting
test_rate_limiting

# Summary
echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║      Load Test Complete!               ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo -e "${YELLOW}Results saved to: ${RESULTS_FILE}${NC}"