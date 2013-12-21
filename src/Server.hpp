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
	bool send_to(const Datagram &dg, const AddrStorage &addr);
	bool receive(Datagram &dg, AddrStorage *addr, int s);

	//Protocole de base
	bool connect_ack(const Datagram &dg, const AddrStorage &addr);
	bool disconnect_ack(const Datagram &dg, const AddrStorage &addr);
	bool get_file(const Datagram &dg, const AddrStorage &addr);
	bool send_file(const Datagram &dg, const AddrStorage &addr);
	
	//Surcouche serveur
	bool process(const Datagram &dg, const AddrStorage &addr);
	bool update_client_map(const AddrStorage &addr);
	bool remove_file(const string &file);

 private :
	int _sockets[MAXSOCK];
	int _n_socks;
	
	bool _run;

	addr_map _client_map;
	
	vector<Exception> _exc;
	
	// Obsolete
	File _curr;
};

#endif
