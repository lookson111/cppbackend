import argparse
import subprocess
import time
import random
import shlex
import os

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)
script_name = os.path.basename(__file__)
script_path = os.path.abspath(__file__).replace(script_name, '')
perf_fullname = script_path + '/perf.data' 
graph_fullname = script_path + '/graph.svg'

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1


def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    return parser.parse_args().server


def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process


def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()


def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)


def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')


server = run(start_server())
print(server.pid)
print(script_path)
perf = run('perf record -o ' + perf_fullname + ' -p ' + str(server.pid))
make_shots()
stop(server)
perf.wait(10)
time.sleep(1)
flame = run('perf script -i ' + perf_fullname + ' | ~/FlameGraph/stackcollapse-perf.pl | ~/FlameGraph/flamegraph.pl > ' + graph_fullname)
flame.wait(10)
time.sleep(1)
print('Job done')
