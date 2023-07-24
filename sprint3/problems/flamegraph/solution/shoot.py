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
perf_fullname = script_path + 'perf.data' 
graph_fullname = script_path + 'graph.svg'
flame_stack = script_path + 'FlameGraph/stackcollapse-perf.pl'
flame_graph = script_path + 'FlameGraph/flamegraph.pl'
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
    #if output != None:
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process

def run_to_file(command, file_name):
    output = open(file_name, 'w')
    proc = run(command, output)
    proc.wait(10)
    output.close()


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
cmd_perf = 'perf record -g -o ' + perf_fullname + ' -p ' + str(server.pid)
print(cmd_perf)
perf = run(cmd_perf)
make_shots()
stop(server)
perf.wait(10)
if not os.path.exists(perf_fullname):
    print('File not exist!')
    exit()
#cmd_flame = 'perf script -f -i ' + perf_fullname + ' | ' + flame_stack + ' | ' + flame_graph + ' > ' + graph_fullname
#flame = run(cmd_flame)
#output = open('perf.results', 'w')
#run('perf script -f -i ' + perf_fullname, output)
#output = open('stack-perf.results', 'w')
#run(flame_stack + ' perf.results', output)
#output = open(graph_fullname, 'w')
#run(flame_graph + ' stack-perf.results', output)
run_to_file('perf script -f -i ' + perf_fullname, 'perf.results')
run_to_file(flame_stack + ' perf.results', 'stack-perf.results')
run_to_file('perf script -f -i ' + perf_fullname, graph_fullname)
#print(cmd_flame)
#flame.wait(10)
#time.sleep(10)
print('Job done')
