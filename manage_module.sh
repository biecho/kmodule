#!/bin/bash

# Usage:
# ./manage_module.sh module_name [all|build|load|unload|clean]

MODULE_NAME="$1"
ACTION="${2:-all}"  # Default action is "all"

BASE_DIR="./modules"  # Assuming all modules are under a directory named "modules"

if [[ -z "$MODULE_NAME" ]]; then
    echo "Please provide a module name as the first argument."
    exit 1
fi

module_dir="$BASE_DIR/$MODULE_NAME"
module_ko="$module_dir/$MODULE_NAME.ko"

if [[ ! -d "$module_dir" ]]; then
    echo "Module directory for $MODULE_NAME not found!"
    exit 3
fi

perform_action() {
    local action="$1"

    case $action in
        build)
            (cd "$module_dir" && make)
            echo "Module $MODULE_NAME built successfully."
            ;;
        load)
            sudo insmod "$module_ko"
            echo "Module $MODULE_NAME loaded successfully."
            ;;
        unload)
            sudo rmmod "$MODULE_NAME"
            echo "Module $MODULE_NAME unloaded successfully."
            ;;
        clean)
            (cd "$module_dir" && make clean)
            echo "Cleaned build artifacts for module $MODULE_NAME."
            ;;
        *)
            echo "Unknown action $action. Supported actions: all|build|load|unload|clean."
            exit 4
            ;;
    esac
}

if [[ "$ACTION" == "all" ]]; then
    for action in build load unload clean; do
        perform_action $action
    done
else
    perform_action $ACTION
fi

