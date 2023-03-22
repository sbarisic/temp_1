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

TODO:

- Postaviti web na server
- API za pristup mikrokontroleru
- Mikrokontroler salje podatke na server
- Server salje podatke na mikrokontroler


* Uprava
* Vozila - Tablica
			* Akumulator 1
			* Akumulator 2
			* Stanje pritiska 1 npr...
			
* Vozila - Tablica
			* Akumulator 1
			* Akumulator 2
			* Stanje pritiska 1 npr...