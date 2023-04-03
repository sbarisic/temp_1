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

Device API

```
{
	"APIVersion": 1,
	"APIKey": "ZJC8cR4XXJyNYh9MKzDkowszMkKqqfxw6fmn5TErkaE=",
	"EquipmentKey": "5xQfOnd__Ea75WWZJ3r2YA",
	"Value": "12.5"
}
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