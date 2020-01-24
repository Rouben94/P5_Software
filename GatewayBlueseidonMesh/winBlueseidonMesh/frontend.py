
import sys

import interactive_pyaci
import time
import os

from argparse import ArgumentParser

from aci.aci_uart import Uart
from aci.aci_utils import STATUS_CODE_LUT
from aci.aci_config import ApplicationConfig
import aci.aci_cmd as cmd
import aci.aci_evt as evt

from mesh import access
from mesh.provisioning import Provisioner, Provisionee  # NOQA: ignore unused import
from mesh import types as mt                            # NOQA: ignore unused import
from mesh.database import MeshDB                        # NOQA: ignore unused import
from models.config import ConfigurationClient           # NOQA: ignore unused import
from models.generic_on_off import GenericOnOffClient    # NOQA: ignore unused import
from models.node_config import NodeConfigClient    # NOQA: ignore unused import

# device = None


def scan_unprovisioned_nodes():
    print("**Scannning unprovisioned nodes")
    time.sleep(1)
    p.scan_start()
    time.sleep(5)
    p.scan_stop()


def provision_node(provision_params):
    print("**Provisioning process started")
    print("**UUID: ", provision_params[1])
    print("**Name: ", provision_params[2])

    time.sleep(1)

    time.sleep(2)

    # device.send(cmd.DevkeyAdd(
    #    db.nodes[0].unicast_address, 0, db.nodes[0].device_key))
    # device.send(cmd.AddrPublicationAdd(db.nodes[0].unicast_address))
    time.sleep(1)

    p.provision(uuid=provision_params[1], name=provision_params[2])


def config_node(nodeconfig_params):
    print("**Configure existing node")
    print("**UUID: ", nodeconfig_params[1])
    print("**NodeID: ", nodeconfig_params[2])
    print("**Channel 1: ", nodeconfig_params[3])
    print("**Channel 2: ", nodeconfig_params[4])
    print("**Channel 3: ", nodeconfig_params[5])
    print("**Channel 4: ", nodeconfig_params[6])
    print("**LPN State: ", nodeconfig_params[7])
    print("**LPN Poll Timeout: ", nodeconfig_params[8])
    print("**LPN Delay: ", nodeconfig_params[9])
    print("**LPN Receive Window: ", nodeconfig_params[10])
    #print("**Unicast Address from UUID: ",db.nodes[int(nodeconfig_params[3])].unicast_address)
    #print("**DevKey from UUID: ",db.nodes[int(nodeconfig_params[3])].device_key)
    time.sleep(1)

    # device.send(cmd.DevkeyAdd(
    #     db.nodes[int(nodeconfig_params[3])].unicast_address, 0, db.nodes[int(nodeconfig_params[3])].device_key))
    # device.send(cmd.AddrPublicationAdd(
    #     db.nodes[int(nodeconfig_params[3])].unicast_address))
    # time.sleep(1)

    global cc
    cc.publish_set(8, 0)
    cc.composition_data_get()
    time.sleep(2)

    cc.appkey_add(0)
    time.sleep(1)
    print("**Appkey Added")

    cc.model_app_bind(
        db.nodes[int(nodeconfig_params[2])].unicast_address, 0, mt.ModelId(0x1002))
    time.sleep(1)
    print("**Appkey Bound")

    time.sleep(2)
    print("**Node Config Client created")

    nc.publish_set(0, 0)
    time.sleep(1)

    # nc.set(nodeconfig_params[4])
    nc.set(int(nodeconfig_params[3]), int(nodeconfig_params[4]), int(nodeconfig_params[5]), int(nodeconfig_params[6]),
           int(nodeconfig_params[7]), int(nodeconfig_params[8]), int(nodeconfig_params[9]), int(nodeconfig_params[10]))
    time.sleep(5)
    print("**Set Params")

    cc.node_reset()
    time.sleep(1)
    print("**Reset Node")


def connect_to_existing_mesh():
    print("**Connect to an existing mesh network!")
    time.sleep(2)

    device.send(cmd.DevkeyAdd(
        db.nodes[0].unicast_address, 0, db.nodes[0].device_key))
    device.send(cmd.AddrPublicationAdd(db.nodes[0].unicast_address))
    time.sleep(1)

    cc = interactive_pyaci.ConfigurationClient(db)
    device.model_add(cc)
    cc.publish_set(8, 0)
    cc.composition_data_get()


def script_startup(input_options_startup):

    global device
    device = interactive_pyaci.start_ipython(input_options_startup)
    global db
    db = interactive_pyaci.MeshDB("database/" + "example_database.json")
    #db = interactive_pyaci.MeshDB(
    #   r"C:\Tools\NRF\nrf5_SDK_for_Mesh_v3.2.0_src\scripts\interactive_pyaci_ohne_IPython\database\example_database.json")
    time.sleep(1)
    global p
    p = interactive_pyaci.Provisioner(device, db)
    global cc
    cc = interactive_pyaci.ConfigurationClient(db)
    device.model_add(cc)
    global nc
    nc = interactive_pyaci.NodeConfigClient()
    device.model_add(nc)
    global g_onoff_client
    g_onoff_client = interactive_pyaci.GenericOnOffClient()
    device.model_add(g_onoff_client)

    print("**ipython started")


def light_switch(light_switch_params):
    print("**Light Switch Status: ", bool(int(light_switch_params[1])))


    g_onoff_client.publish_set(0, 0)
    print("**gofc set")
    g_onoff_client.set(bool(int(light_switch_params[1])))
    time.sleep(2)

    print("**Set value to: ", bool(int(light_switch_params[1])))


def subscription_add(subscription_params):
    print("**UUID: ", subscription_params[1])
    print("**NodeID: ", subscription_params[2])
    print("**Group Number : ", subscription_params[3])

    global cc
    cc.publish_set(8, 0)
    cc.composition_data_get()
    time.sleep(2)

    cc.appkey_add(0)
    time.sleep(1)
    print("**Appkey Added")

    cc.model_app_bind(
        (db.nodes[int(subscription_params[2])].unicast_address) + 1, 0, mt.ModelId(0x1000))
    time.sleep(1)
    print("**Appkey Bound")

    cc.model_subscription_add(db.nodes[int(subscription_params[2])].unicast_address + 1,
                              db.groups[int(subscription_params[3])].address, mt.ModelId(0x1000))


def publication_add(publication_params):
    print("**UUID: ", publication_params[1])
    print("**NodeID: ", publication_params[2])
    print("**Group Number : ", publication_params[3])

    global cc
    cc.publish_set(8, 0)
    cc.composition_data_get()
    time.sleep(2)

    cc.appkey_add(0)
    time.sleep(1)
    print("**Appkey Added")

    cc.model_app_bind(
        (db.nodes[int(publication_params[2])].unicast_address) + 1, 0, mt.ModelId(0x1000))
    time.sleep(1)
    print("**Appkey Bound")

    cc.model_publication_set(db.nodes[int(publication_params[2])].unicast_address + 1, mt.ModelId(
        0x1000), mt.Publish(db.groups[int(publication_params[3])].address, index=0, ttl=1))


if __name__ == '__main__':
    parser = ArgumentParser(
        description="nRF5 SDK for Mesh Interactive PyACI")
    parser.add_argument("-d", "--device",
                        dest="devices",
                        nargs="+",
                        required=False,
                        help=("Device Communication port, e.g., COM216 or "
                              + "/dev/ttyACM0. You may connect to multiple "
                              + "devices. Separate devices by spaces, e.g., "
                                + "\"-d COM123 COM234\""))

    """ parser.add_argument("-m", "--mode",
                        dest="mode",
                        type=int,
                        required=False,
                        default=False,
                        help="Set the mode of the gateway interface") """

    parser.add_argument("-b", "--baudrate",
                        dest="baudrate",
                        required=False,
                        default='115200',
                        help="Baud rate. Default: 115200")
    parser.add_argument("--no-logfile",
                        dest="no_logfile",
                        action="store_true",
                        required=False,
                        default=False,
                        help="Disables logging to file.")
    parser.add_argument("-l", "--log-level",
                        dest="log_level",
                        type=int,
                        required=False,
                        default=3,
                        help=("Set default logging level: "
                              + "1=Errors only, 2=Warnings, 3=Info, 4=Debug"))
    input_options = parser.parse_args()

    input_options.devices = ["COM3"]

    for line in sys.stdin:
        line = line.strip()
        # print(line)

        if line == "script_startup":
            print("**Test startup")
            script_startup(input_options)
            print("**Script Startup")

        if line == "connect_to_mesh":
            connect_to_existing_mesh()
            print("**Connection established")

        if line == "scan":
            scan_unprovisioned_nodes()
            print("**Scan finished")

        if "provision_node" in line.split():
            provision_params = list(line.split(" "))
            provision_node(provision_params)
            time.sleep(1)
            print("**Node successfully provisioned")

        if "config_node" in line.split():
            nodeconfig_params = list(line.split(" "))
            config_node(nodeconfig_params)
            time.sleep(1)
            print("**Node successfully configured")

        if "light_switch" in line.split():
            light_switch_params = list(line.split(" "))
            light_switch(light_switch_params)
            time.sleep(1)
            print("**Light successfully set ", light_switch_params[3])

        if "subscription_add" in line.split():
            subscription_params = list(line.split(" "))
            subscription_add(subscription_params)
            time.sleep(1)
            print("**Subscription successfully added")

        if "publication_add" in line.split():
            publication_params = list(line.split(" "))
            publication_add(publication_params)
            time.sleep(1)
            print("**Publication successfully added")
