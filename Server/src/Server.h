/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 * Classe Server : gestion de la connexion avec les clients
 */

#include "socket.h"
#include "File.h"
#include "AddrStorage.h"
#include "Converter.h"

using namespace std;

class Server
{
 private :
	File* _log;

	int _sockets[MAXSOCK];
	int _n_socks;
	int _r;
	
	string _port;

	struct addrinfo _local, *_iterator, *_start;


	//Futur classe client
	struct sockaddr_storage _client_addr;
	socklen_t _client_len;
	
	AddrStorage* _addr;

 public :
	Server(string port);
	~Server();
	bool connect();
	bool receive(int s);
	bool send_to(Datagram* dg, AddrStorage* addr);

	//Protocole de base
	void toctoc(AddrStorage* addr);
	
	//Surcouche serveur
};
