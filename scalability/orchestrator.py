from dataclasses import dataclass
import json
import os
import io
import sys
import subprocess
import collections
import logging
import shutil
import shlex

import threading
import time

from typing import Union

logger = logging.getLogger(__name__)

class PrintTracerThread(threading.Thread):
    def __init__(self, job, should_kill):
        threading.Thread.__init__(self)
        self.should_kill = should_kill
        self.job = job

    def run(self):
        if self.job is None:
            return

        logger.info("Starting tracer logger")

        while True:
            time.sleep(1)
            return_code = self.job.process.poll()

            if return_code is None:
                if self.job.process.stdout is not None:
                    for line in self.job.process.stdout:
                        line = line.strip()
                        if line and not line.isspace():
                            print(line)

                if self.job.process.stderr is not None:
                    for line in self.job.process.stderr:
                        line = line.strip()
                        if line and not line.isspace():
                            print(line)
            else:
                if return_code == 0:
                    print("Tracer Completed Successfully")
                    return 0
                else:
                    if return_code == -9:
                        print("Tracer Terminated (-9)")
                    elif return_code != 0:
                        print("Tracer Failed ({})".format(return_code))
                    else:
                        print("Tracer Failed with Unknown return_code")
                    return -1

class CheckStatusThread(threading.Thread):
    def __init__(self, process_list, should_kill):
        threading.Thread.__init__(self)
        self.should_kill = should_kill
        self._process_list = process_list
        self._status = {}

    def run(self):
        logger.info("Starting checks logger")
        for proc in self._process_list:
            self._status[proc.name] = 0

        has_failed = False
        while True:
            time.sleep(1)

            for proc in self._process_list:
                self.status(proc)
                if has_failed and self.should_kill:
                    proc.process.kill()

                if proc.process.poll() is not None and proc.process.returncode != 0:
                    self._status[proc.name] = proc.process.returncode
                    if has_failed is False and self.should_kill is True:
                        print("Process {} has failed, killing other processes".format(proc.name))
                    has_failed = True

            all_procs = [proc.process.poll() for proc in self._process_list if proc.process.poll() is not None]
            if len(all_procs) == len(self._process_list):
                if all(proc ==0 for proc in all_procs):
                    return 0
                else:
                    return -1
            else:
                continue

    def status(self, proc):
        if proc.process.poll() is None:
            status = "running"
        elif proc.process.returncode == 0:
            status = "success"
        elif proc.process.returncode == -9:
            status = "terminated"
        elif proc.process.returncode != 0:
            status = "failed"
        else:
            status = "unknown"

        logger.info("Process '{}': {}".format(proc.name, status))

@dataclass
class Job:
    name: str
    process: subprocess.Popen
    file:str

@dataclass
class Output:
    name: str
    file: Union[io.TextIOWrapper, None]

def add_broker(config):
    port = 23500
    # core_type = "zmq"

    if "broker" in config.keys():
        if "port" in config["broker"].keys():
            port = config["broker"]["port"]

        # if "coreType" in config["broker"].keys():
        #     core_type = config["broker"]["coreType"]

    config["federates"].append(
        {
            "directory": ".",
            "exec": "helics_broker --federates={} --localport={}".format(len(config["federates"]), port),
            "host": "localhost",
            "name": "broker"
        }
    )

def setup_federates(config, logging_path, root_dir):
    process_list = []
    output_list = []
    for fed in config["federates"]:
        print("Running federate {} as a background process".format(fed["name"]))

        fname = open(os.path.abspath(os.path.join(logging_path, "{}.log".format(fed["name"]))), "w")

        try:
            directory = os.path.join(root_dir, fed["directory"])

            env = dict(os.environ)
            if "env" in fed:
                for k, v in fed["env"].items():
                    env[k] = v

            p_args = shlex.split(fed["exec"])
            p_args[0] = shutil.which(p_args[0]);
            if p_args[0] is None:
                print("Command '{}' specified in exec string is not recognized.".format(fed["exec"]))
                return

            proc = subprocess.Popen(
                p_args,
                cwd=os.path.abspath(os.path.expanduser(directory)),
                stdout=fname,
                stderr=fname,
                env=env
            )

            job = Job(fed["name"], proc, fname)

        except FileNotFoundError as e:
            print("FileNotFoundError: {}".format(e))

        process_list.append(job)
        if fname is not None:
            output_list.append(fname)

    return process_list, output_list

def setup_tracer(config, logging_path, root_dir):
    if "tracer" in config.keys():
        tracer = config["tracer"]

        directory = root_dir
        if "directory" in tracer.keys():
            directory = os.path.join(root_dir, tracer["directory"])

        env = dict(os.environ)
        if "env" in tracer:
            for k, v in tracer["env"].items():
                env[k] = v

        tracer_name = "tracer"
        if "name" in tracer.keys():
            tracer_name = tracer["name"]
        fname = open(os.path.abspath(os.path.join(logging_path, "{}.log".format(tracer_name))), "w")

        print("Running tracer '{}' as a background process".format(tracer_name))

        tracer_config = "tracer_config.json"
        if "config" in tracer.keys():
            tracer_config = tracer["config"]
        tracer_config = os.path.abspath(os.path.join(logging_path, tracer_config))

        exec_string = shlex.split("helics_app tracer -o {} --config-file {}".format(fname.name, tracer_config))
        exec_string[0] = shutil.which(exec_string[0]);
        proc = subprocess.Popen(
            exec_string,
            cwd=os.path.abspath(os.path.expanduser(directory)),
            stdout=fname,
            stderr=fname,
            env=env
        )

        job = Job(tracer_name, proc, fname)

        return job

def execute(process_list, output_list, tracer_job):
    status_thread = CheckStatusThread(process_list=process_list, should_kill=True)
    tracer_thread = PrintTracerThread(tracer_job, should_kill=True)

    try:
        status_thread.start()
        tracer_thread.start()
        print("Waiting for {} processes to finish".format(len(process_list) + 1))

    except KeyboardInterrupt:
        print("User requested termination. Shutting down safely...")

        for file_handle in output_list:
            file_handle.close()

        for job in process_list:
            job.process.kill()

        tracer_job.file.close()
        tracer_job.process.kill()

    except Exception as e:
        print("Error: {}".format(e))
        print("Terminating...")

        for file_handle in output_list:
            file_handle.close()

        for job in process_list:
            job.process.kill()

        tracer_job.file.close()
        tracer_job.process.kill()

    finally:
        errored = False
        for proc in process_list:
            status_thread.status(proc)
            if (
                proc.process.returncode != 0
                and proc.process.returncode != -9
                and proc.process.returncode is not None
            ):
                errored = True
                print("Process {} exited with return code {}".format(proc.name, proc.process.returncode))
                if os.path.exists(proc.file):
                    with open(proc.file) as f:
                        print("Last 10 lines of {}.log".format(proc.name))
                        print("...")
                        for line in f.readlines()[-10]:
                            print(line, end="")
                        print("...")

        if not errored and tracer_job is not None:
            errored = tracer_job.process.returncode != 0
            if errored:
                print("Tracer failed with return code {}".format(tracer_job.process.returncode))

        return errored

def run(json_input):
    path_to_config = os.path.abspath(json_input)
    if not os.path.exists(path_to_config):
        print("Unable to find file '{}'".format(path_to_config))
        print("Returning...")
        return

    with open(path_to_config) as json_file:
        config = json.loads(json_file.read())

    print("Running Federation: {}".format(config["name"]))

    # Add broker, set defaults and check for inputs
    add_broker(config)

    # check for duplicate names
    names = [c["name"] for c in config["federates"]]
    if len(set(name for name in names)) != len(config["federates"]):
        print("Error: Reapeated names found in json file federates.")
        for name, c in [
            (item, count)
            for item, count in collections.Counter(names).items()
            if count > 1
        ]:
            print("Found name '{}' {} times".format(name, c))
        return

    # Get logging path
    root_dir = os.path.dirname(path_to_config)
    logging_path = root_dir
    if "logging_path" in config.keys():
        logging_path = config["logging_path"]

    # Setup Federates
    process_list, output_list = setup_federates(config, logging_path, root_dir)

    # Setup Tracer
    job = setup_tracer(config, logging_path, root_dir)
    # process_list.append(job)
    # if job.file is not None:
    #     output_list.append(job.file)

    errored = execute(process_list, output_list, job)
    if errored:
        sys.exit()

    print("Done.")

def run_subprocess(env, directory, exec_string, name):
    with open("{}.log".format(name), "w") as log:
        proc = subprocess.Popen(
            exec_string,
            cwd=directory,
            env=env
        )

        jobs = []
        job = Job(name, proc, log)
        jobs.append(job)
        try:
            t = CheckStatusThread(jobs, True)
            t.start()
        except Exception as e:
            print("Error when running test {}\nException: {}".format(job.name, e))

def test_environment(json_input):
    path_to_config = os.path.abspath(json_input)
    if not os.path.exists(path_to_config):
        print("Unable to find file '{}'".format(path_to_config))
        print("Returning...")
        return

    with open(path_to_config) as json_file:
        config = json.loads(json_file.read())

    root_dir = os.path.dirname(path_to_config)

    if "tracer" in config.keys():
        tracer = config["tracer"]

        directory = root_dir
        if "directory" in tracer.keys():
            directory = os.path.join(root_dir, tracer["directory"])
        directory = os.path.abspath(os.path.expanduser(directory))

        env = dict(os.environ)
        if "env" in tracer:
            for k, v in tracer["env"].items():
                env[k] = v

        exec_string = shlex.split("echo $PATH")
        exec_string[1] = os.path.expandvars(exec_string[1])
        run_subprocess(env, directory, exec_string, "testbed_1")

        exec_string = shlex.split("echo $PWD")
        exec_string[1] = os.path.expandvars(exec_string[1])
        run_subprocess(env, directory, exec_string, "testbed_2")

        exec_string = shlex.split("ls /home/shelf1/compile/lwilliamson/cosim-web-app/server/uncc-corvid/scalability/build/deploy/tracer.log")
        run_subprocess(env, directory, exec_string, "testbed_3")

        exec_string = shlex.split("ls /home/shelf1/compile/lwilliamson/cosim-web-app/server/uncc-corvid/scalability/build/deploy/tracer_config.json")
        run_subprocess(env, directory, exec_string, "testbed_4")

def main(argv):
    if len(argv) < 2:
        print("Please provide an input json file.")
        return

    run(argv[1])
    # test_environment(argv[1])
    return

if __name__ == "__main__":
    main(sys.argv)
