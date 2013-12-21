/*
 * Alexandre Kornmann
 * Projet reseau
 * Master 2 CSMI
 * Classe Server : gestion de la connexion avec les clients
 */

#include "socket.hpp"
#include "File.hpp"
#include "AddrStorage.hpp"
#include "Converter.hpp"
#include "State.hpp"
#include "addr_map.hpp"
#include "Datagram.hpp"
#include "Exception.hpp"
#include <vector>

using namespace std;

class Server
{
 public :
	Server(string port);
	~Server();
	
	int sock(const AddrStorage &addr);
	void run();
	
	//Envoi/reception de datagrammes
	bool send_to(const Datagram &dg, const AddrStorage &addr);
	bool receive(Datagram &dg, AddrStorage *addr, int s);

	//Protocole de base
	bool toctoc(Datagram &dg, const AddrStorage &addr);
	bool get_file(const Datagram &dg, const AddrStorage &addr);
	bool send_file(const Datagram &dg, const AddrStorage &addr);
	
	//Surcouche serveur
	bool process(Datagram &dg, const AddrStorage &addr);
	bool update_client_map(const AddrStorage &addr);

 private :
	int _sockets[MAXSOCK];
	int _n_socks;
	
	bool _run;

	addr_map _client_map;
	
	vector<Exception> _exc;
	
	// Obsolete
	File _curr;
};
