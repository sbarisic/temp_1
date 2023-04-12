# temp_1

Ports

```
sudo firewall-cmd --permanent --add-port=PORT_NUM/udp
sudo firewall-cmd --reload
```

Certificate

```
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /etc/ssl/private/demo-selfsigned.key -out /etc/ssl/certs/demo-selfsigned.crt
sudo openssl pkcs12 -inkey /etc/ssl/private/demo-selfsigned.key -in /etc/ssl/certs/demo-selfsigned.crt -export -out demoCA.pfx
```

Nginx

```
sudo micro /etc/nginx/nginx.conf
sudo micro /etc/nginx/sites-available/default
sudo nginx -t

sudo systemctl restart nginx  
```

Service
```
sudo systemctl enable dotnet_server.service

sudo systemctl stop dotnet_server.service
sudo systemctl start dotnet_server.service
sudo systemctl status dotnet_server.service

sudo systemctl restart dotnet_server.service  

# logs
sudo journalctl -fu dotnet_server.service
```

Database

```
# listen_addresses = '*'
sudo micro /etc/postgresql/*/main/postgresql.conf

sudo systemctl restart postgresql
```


```
TODO:
- JSON API za pristup mikrokontroleru


* Uprava
* Vozila - Tablica
			* Akumulator 1
			* Akumulator 2
			* Stanje pritiska 1 npr...
			
* Vozila - Tablica
			* Akumulator 1
			* Akumulator 2
			* Stanje pritiska 1 npr...
```


#### Device API

```json
// Postavi vrijednosti
{
	"APIVersion": 1,
	"APIKey": "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=",
	"Action": 1,
	"EquipmentKey": "7D-K0CG9rEW2iEwViKHqAg",
	"Value": "12.0"
}


// Dohvati svu opremu za API key
{
	"APIVersion": 1,
	"APIKey": "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=",
	"Action": 2
}
```