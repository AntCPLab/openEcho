from task_runner import TaskOrchestrator, Task, BuiltinFilters
import re
import os

SYNC_CONFIG = {
    "serve_barrier": os.environ["ID"] == '0',
    "sync_host": os.environ["P0"],
    "sync_port": 9999,
    "num_participants": 3,
}

executables = [
    # "replicated-ring-party.x",
    # "echo-prep-party.x",
    # "echo-party.x",
    # "astra-prep-party.x",
    # "astra-party.x",
    # "sy-rep-ring-party.x",
    # "sy-abfl.x"
    "covert-mac-party.x"
]

circuits = [
    # "echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split",
    # "echo-tf-EzPC_Athos_Networks_ResNet_graphDef.bin-1-trunc_pr-split",
    # "echo-mnist_full_A-1-128-16-trunc_pr-split-no_acc",
    # "echo-mnist_full_A-1-128-1-trunc_pr-split-no_acc",
    # "echo-mnist_full_D-1-128-16-2dense-trunc_pr-split-no_acc",
    # "echo-mnist_full_D-1-128-1-2dense-trunc_pr-split-no_acc",
    # "echo-mnist_full_B-1-128-16-debug5000-trunc_pr-split-no_acc",
    # "echo-mnist_full_B-1-128-1-debug5000-trunc_pr-split-no_acc",
    # "echo-mnist_full_C-1-128-16-debug5000-trunc_pr-split-no_acc",
    # "echo-mnist_full_C-1-128-1-debug5000-trunc_pr-split-no_acc",
    # "tf-EzPC_Athos_Networks_ResNet_graphDef.bin-16-trunc_pr-split",
    # "mnist_full_A-1-128-16-trunc_pr-split-no_acc",
    # "mnist_full_B-1-128-16-debug5000-trunc_pr-split-no_acc",
    # "mnist_full_C-1-128-16-debug5000-trunc_pr-split-no_acc",
    # "mnist_full_D-1-128-16-2dense-trunc_pr-split-no_acc",
    "52-mnist_full_A-1-128-16-split-no_acc",
    "52-mnist_full_B-1-128-16-debug5000-split-no_acc",
    "52-mnist_full_C-1-128-16-debug5000-split-no_acc",
    "52-mnist_full_D-1-128-16-2dense-split-no_acc",
]

extra_args = f"-b 1048576 -q 16 -lm 2 -S 12 -h {os.environ['P0']} -pn 12345"
# extra_args += " -o nofile"

def build_resnet_tasks():
    tasks = []
    tasks.append(
        Task(
            name = 'prepare input resnet',
            commands = [
                "cp EzPC/Athos/Networks/ResNet/ResNet_img_input.inp Player-Data/Input-Binary-P0-0 ",
                "sleep 1"
                "sleep 1"
            ]
        )
    )
    for circuit in circuits:
        if 'ResNet' not in circuit:
                continue
        for executable in executables:
            if 'astra-party' in executable:
                commands = [
                    "sleep 1",
                    f"./{executable} 0 {circuit} -h {os.environ['P1']} -pn 12345",
                    f"./{executable} 1 {circuit} -h {os.environ['P1']} -pn 12345",
                ]
            else:
                commands = [
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                ]
            tasks.append(
                Task(
                    name = f"{executable}-{circuit}",
                    commands = commands
                )
            )
    return tasks

def build_mnist_full_tasks():
    tasks = []
    tasks.append(
        Task(
            name = 'prepare input mnist full',
            commands = [
                "python3 prepare_mnist_data.py",
                "sleep 1",
                "sleep 1"
            ]
        )
    )
    for circuit in circuits:
        if 'mnist' not in circuit:
            continue
        if 'debug5000' in circuit:
            continue
        for executable in executables:
            if 'astra-party' in executable:
                commands = [
                    "sleep 1",
                    f"./{executable} 0 {circuit} -h {os.environ['P1']} -pn 12345",
                    f"./{executable} 1 {circuit} -h {os.environ['P1']} -pn 12345",
                ]
            else:
                commands = [
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                ]
            tasks.append(
                Task(
                    name = f"{executable}-{circuit}",
                    commands = commands
                )
            )
    return tasks

def build_mnist_debug_tasks():
    tasks = []
    tasks.append(
        Task(
            name = 'prepare input mnist debug',
            commands = [
                "python3 prepare_mnist_data.py 5000 5000",
                "sleep 1",
                "sleep 1"
            ]
        )
    )
    for circuit in circuits:
        if 'mnist' not in circuit:
            continue
        if 'debug5000' not in circuit:
            continue
        for executable in executables:
            if 'astra-party' in executable:
                commands = [
                    "sleep 1",
                    f"./{executable} 0 {circuit} -h {os.environ['P1']} -pn 12345",
                    f"./{executable} 1 {circuit} -h {os.environ['P1']} -pn 12345",
                ]
            else:
                commands = [
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                    f"./{executable} {os.environ['ID']} {circuit} {extra_args}",
                ]
            tasks.append(
                Task(
                    name = f"{executable}-{circuit}",
                    commands = commands
                )
            )
    return tasks

def build_tasks():
    tasks = []
    tasks.extend(build_resnet_tasks())
    tasks.extend(build_mnist_full_tasks())
    tasks.extend(build_mnist_debug_tasks())
    return tasks

tasks = build_tasks()

orchestrator = TaskOrchestrator(
    log_dir="./logs/NN-experiments",
    repeats=1,
    command_filter=lambda party_id, cmd: party_id == int(os.environ["ID"]),
    **SYNC_CONFIG,
)

orchestrator.run(tasks)