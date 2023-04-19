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
ESP32 https://platformio.org/
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

TODO: Izvor greske na web ekranu

Podaci:
Vozilo - broj vozila
Timestamp - 
Napon 1
Napon 2
Temperatura
Tlak
Voltage Drop - Najvise izmjereni pad napona na akumulatoru
Error message -

Spremanje JSON podataka u tablicu
```


## Device API

![alt text](https://raw.githubusercontent.com/sbarisic/temp_1/master/screenshots/a.png "A")

REST POST na ``/deviceaccess`` sa JSON podacima u bodyju

Postavljanje vrijednosti opreme

Request
```json
{
	"APIVersion": 1,
	"APIKey": "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=",
	"Action": 1,
	"EquipmentKey": "7D-K0CG9rEW2iEwViKHqAg",
	"Value": "12.4"
}
```

Response
```json
{
	"title": "DeviceAccessResponseAPI",
	"status": 200,
	"statusString": "OK"
}
```

Dohvat sve opreme za APIKey

Request
```json
{
	"APIVersion": 1,
	"APIKey": "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=",
	"Action": 2
}
```

Response
```json
{
	"title": "DeviceAccessResponseAPI",
	"status": 200,
	"statusString": "OK",
	"equipment": [
		{
			"id": "7D-K0CG9rEW2iEwViKHqAg",
			"name": "Akumulator 1",
			"equipmentType": 1
		},
		{
			"id": "Qhdk1g4PP0m7T4DKz3anGQ",
			"name": "Akumulator 2",
			"equipmentType": 1
		}
	]
}
```

Definicija ``Action`` polja
https://github.com/sbarisic/temp_1/blob/main/ASP_Web/Proj2/Code/DeviceAccessController.cs#L16

Definicija ``equipmentType`` polja
https://github.com/sbarisic/temp_1/blob/main/ASP_Web/Proj2/Database/DbTables.cs#L515



```json
{
    "APIKey": "OoDUEAxaDLE3L+tdG2ZWmvSNJ8A5jnzh9a4r4d4XzEw=_",
    "A": "string",
    "B": 123,
    "C": { "Value": "testval" },
    "D": [ 1, 2, 3, 4, 5 ],
    "E": true,
    "F": null
}
```