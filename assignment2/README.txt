All the servers will connect to port --> 9158


** TCP (Connection oriented)**
Connection Type:
PERSISTENT - 1
NONPERSISTENT - 2

For single process server:
./single_server

For multithreaded server:
./thread_server


Client execution for single and multithreaded server:
./client server_name 9158 connection_type filenames

Example
./client localhost 9158 2 1mb.txt 1mb.txt 1mb.txt 1mb.txt


** UDP (Connection less) **
For server:
./udp_server

For client:
./udp_client server_name 9158 filename
