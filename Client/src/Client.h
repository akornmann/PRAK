/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 * Classe Client : gestion de la connexion avec le serveur
 */

#include "AddrStorage.h"
#include "File.h"

using namespace std;

class Client
{
 private :
	struct sockaddr_storage _server_addr;
	socklen_t _server_len;
	struct timeval _time_val;

	AddrStorage* _server;

	int _socket;

	int _r;
	File* _conf;
	File* _log;
	string _error;
	Status _status;

 public :
	Client();
	~Client();
	bool connect(string addr, string port);
	bool disconnect();
	bool send_to(Datagram* dg, AddrStorage* addr);
	bool receive_from(Datagram* dg, AddrStorage* addr);

	//Protocole de base
	void toctoc(AddrStorage* addr);

	//Surcouche client
	//Protocole toctoc
	bool do_toctoc();
	bool server_select();

	//Protocole file
	bool do_file(string file);


	//setter/getter
	void set_conf(string cfg) { _conf->set_file(cfg); }
	void set_error(string e) { _error = e; }
	void set_status(Status s) { _status = s; }
	string error() { return _error; }
	string status();
};
