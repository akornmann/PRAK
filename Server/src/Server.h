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

 public :
	Server(string port);
	~Server();
	bool connect();
	bool disconnect();
	
	//Gestion des datagrammes
	bool init(Datagram* dg, int code, int seq, string s);
	bool init(Datagram* dg, int code, int seq);

	//Envoi/reception de datagrammes
	bool receive(int s);
	bool send_to(Datagram* dg, AddrStorage* addr);

	//Protocole de base
	bool toctoc(Datagram* dg, AddrStorage* addr);
	bool get_file(string file, AddrStorage* addr);
	bool send_file(string file, AddrStorage* addr);
	
	//Surcouche serveur
	bool process(Datagram* dg, AddrStorage* addr);
	bool get_file(string file);
	bool send_file(string file);
};
