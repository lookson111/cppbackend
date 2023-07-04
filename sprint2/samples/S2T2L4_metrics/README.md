# Используемые команды
 
wget https://github.com/prometheus/node_exporter/releases/download/v1.4.0/node_exporter-1.4.0.linux-amd64.tar.gz
tar xvzf node_exporter-1.4.0.linux-amd64.tar.gz
cd node_exporter-1.4.0.linux-amd64/
./node_exporter
sudo ufw allow 9100/tcp
nohup ./node_exporter &
sudo docker run -d --name prom --network="host" prom/prometheus
sudo ufw allow 9090/tcp
sudo docker run --network="host" prom/prometheus
mkdir prom
cd prom/
vim prometheus.yml
sudo docker cp prometheus.yml prom:/etc/prometheus
sudo docker restart prom
nohup ./node_exporter &
sudo docker run -d --name=grafana --network="host" grafana/grafana
pip install prometheus_client
vim web_exporter.py
python3 web_exporter.py

