all flash clean:
	$(MAKE) -C firmware $@

test:
	$(MAKE) -C chat test

.PHONY: all flash clean test
