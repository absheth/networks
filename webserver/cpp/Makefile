make:
	g++ -g -std=c++11 -pthread thread_server.cpp -o thread_server
	g++ -g -std=c++11 single_server.cpp -o single_server
	g++ -g -std=c++11 client.cpp -o client
	g++ -g -std=c++11 udp_server.cpp -o udp_server
	g++ -g -std=c++11 udp_client.cpp -o udp_client

clean:
	rm -rf thread_server
	rm -rf single_server
	rm -rf client
	rm -rf udp_server
	rm -rf udp_client
	rm -rf *.dSYM
