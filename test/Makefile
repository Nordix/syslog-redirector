
.PHONY: all clean

all: test_uds test_file test_syslog
	@echo "Running tests..."
	LD_PRELOAD=../syslog-redirector.so timeout 5s ./test_uds > /dev/null
	LD_PRELOAD=../syslog-redirector.so timeout 5s ./test_file > /dev/null
	LD_PRELOAD=../syslog-redirector.so SYSLOG_PATH=/dev/stdout timeout 5s ./test_syslog | grep -q "test message" > /dev/null
	LD_PRELOAD=../syslog-redirector.so SYSLOG_PATH=file:/dev/stderr timeout 5s ./test_syslog 2>&1 | grep -q "test message" > /dev/null
	@echo "All tests passed."

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f test_uds test_file test_syslog
