#!/bin/bash

echo "Testing with domains that exist in database..."

domains=("google.com" "facebook.com" "github.com" "stackoverflow.com")
success_count=0
total_time=0

# Function to test a single client
test_client() {
    local i=$1
    local domain=${domains[$((i % 4))]}
    local start_time=$(date +%s%N)
    
    local response=$(echo "1:$domain" | nc -w 5 127.0.0.1 8081 2>/dev/null)
    
    local end_time=$(date +%s%N)
    local duration=$(( (end_time - start_time) / 1000000 ))
    
    if [[ -n "$response" && "$response" != "NOT_FOUND" && "$response" != "DNS_SERVER_ERROR" ]]; then
        echo "Client $i ($domain): SUCCESS - $response (${duration}ms)"
        echo "SUCCESS" > /tmp/client_$i.result
        echo "$duration" > /tmp/client_$i.time
    else
        echo "Client $i ($domain): FAILED or NOT_FOUND - Response: '$response'"
        echo "FAILED" > /tmp/client_$i.result
    fi
}

# Clean up previous results
rm -f /tmp/client_*.result /tmp/client_*.time

echo "Starting 100 concurrent clients..."
start_time=$(date +%s)

# Launch all clients in parallel
for i in {1..100}; do
    test_client $i &
done

# Wait for all background processes
wait

# Count results
for i in {1..100}; do
    if [[ -f /tmp/client_$i.result ]]; then
        result=$(cat /tmp/client_$i.result)
        if [[ "$result" == "SUCCESS" ]]; then
            success_count=$((success_count + 1))
            if [[ -f /tmp/client_$i.time ]]; then
                time_val=$(cat /tmp/client_$i.time)
                total_time=$((total_time + time_val))
            fi
        fi
    fi
done

end_time=$(date +%s)
duration=$((end_time - start_time))

echo ""
echo "=== TEST RESULTS ==="
echo "Total clients: 100"
echo "Successful requests: $success_count"
echo "Failed requests: $((100 - success_count))"
echo "Success rate: $((success_count * 100 / 100))%"
echo "Total test time: ${duration} seconds"

if [ $success_count -gt 0 ]; then
    echo "Average response time: $((total_time / success_count))ms"
fi

# Clean up
rm -f /tmp/client_*.result /tmp/client_*.time
