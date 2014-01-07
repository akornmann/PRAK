#ifndef SERVER_H
#define SERVER_H

#include "Client.hpp"

using namespace std;

class Server : public Client
{
 public :
	Server(string port, string config);
	~Server();

	int sock(const AddrStorage &addr);
	void run();
	
	//Envoi/reception de datagrammes
	void send_to(const Datagram &dg, const AddrStorage &addr);
	void receive(Datagram &dg, AddrStorage *addr, int s);

	//Protocole de base
	void connect_ack(const Datagram &dg, const AddrStorage &addr);
	void disconnect_ack(const Datagram &dg, const AddrStorage &addr);

	void get_file(const Datagram &dg, const AddrStorage &addr);
	void send_file(const Datagram &dg, const AddrStorage &addr);

	void add_file(const Datagram &dg, const AddrStorage &addr);
	void remove_file(const Datagram &dg, const AddrStorage &addr);

	void send_library(const Datagram &dg, const AddrStorage &addr);

	//Surcouche serveur
	void process(const Datagram &dg, const AddrStorage &addr);
	void update_client_map(const AddrStorage &addr);

	int find_file(const string& file);

 private :
	int _sockets[MAXSOCK];
	int _n_socks;
	
	bool _run;

	addr_map _client_map;
	library _lib;
};

#endif
