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
	AddrStorage* _server;

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

	//Gestion des datagrammes
	bool init(Datagram* dg, int code, int seq, string s);
	bool init(Datagram* dg, int code, int seq);	
	void show(string prefix, Datagram* dg);

	//Envoi/reception de datagrammes
	bool send_to(Datagram* dg, AddrStorage* addr);
	bool receive_from(Datagram* dg, AddrStorage* addr);

	//Protocole de base
	bool toctoc(Datagram* dg, AddrStorage* addr);
	bool get_file(string file, AddrStorage* addr);

	//Surcouche client
	bool is_connect();
	bool server_select();
	bool get_file(string file);

	//setter/getter
	void set_conf(string cfg) { _conf->set_file(cfg); }
	string error() { return _error; }
	string status();
};
