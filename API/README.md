These files contains code in NGINX and Flask that were developed that would create threads, messages, and responses. Essentially, these are two API endpoints that 
simplifies the calling of the OpenAI API endpoints. Additionally, we also added the code that would create the service for gunicorn. 
Note that we used AWS EC2 server to create the two API. 

Creating the service for Gunicorn (Ensure you have created a Python virtual environment):
```bash
[Unit]
Description=Gunicorn instance for Voice Assistant Project
After=network.target
[Service]
User=ubuntu
Group=www-data
WorkingDirectory=/home/ubuntu/TestingAPI
ExecStart=/home/ubuntu/TestingAPI/venv/bin/gunicorn -b localhost:8000 app:app
Restart=always
[Install]
Want
```

This is the setup for NGINX (Ensure you have NGINX; the endpoint this scenario is https):
```bash
upstream API{
        server 127.0.0.1:1234;
}
server {

        listen 443 ssl;
        listen [::]:443 ssl;
        #listen 443 ssl;

        ssl_certificate /etc/letsencrypt/live/api.com/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/api.com/privkey.pem;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_prefer_server_ciphers off;
        #include /etc/letsencrypt/options-ssl-nginx.conf;
    

        root /var/www/html;

        index index.html index.htm index.nginx-debian.html;

        #server_name _;

        server_name someapi.com ;

         # Add HSTS header
        add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;

         # Add X-Content-Type-Options header
        add_header X-Content-Type-Options "nosniff" always;

        # Add Referrer-Policy header
        add_header Referrer-Policy "no-referrer-when-downgrade" always;

        # Add Content-Security-Policy header
        #add_header Content-Security-Policy "default-src 'self'; style-src 'self' 'nonce-{nonce}'; script-src 'self' 'nonce-{nonce}'; object-src 'none'; base-uri 'none';" always;

        # Add X-Frame options - sets frame or iframe permission
        add_header X-Frame-Options "SAMEORIGIN" always;

        # Set permission policy
        add_header Permissions-Policy "geolocation=(self), microphone=(), camera=()" always;

        # Hide the default server header
        proxy_hide_header Server;

        # Set your custom server header
        more_set_headers  "Server SomeServer";


        location /API/may/createThread {
                proxy_pass http://API/createThread;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
        }

        location /API/may/genResponse {
                proxy_pass http://API/getResponse;
                proxy_set_header Host $host;
                proxy_set_header X-Real-IP $remote_addr;
                proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                proxy_set_header X-Forwarded-Proto $scheme;
        }
}
```
```
