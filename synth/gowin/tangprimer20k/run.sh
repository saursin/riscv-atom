BUILD_DIR=build
FLOW_STEPS='none'

VERILOG_FILES="$BUILD_DIR/*.v"
TOPMODULE="HydrogenSoC"

CONSTRAINT_FILE="${TOPMODULE}.cst"

FAMILY=
DEVICE='GW2A-LV18PG256C8/I7'    # change to your device
BOARD='tangprimer20k'           # change to your board

# Read cli args
for arg in "$@"; do
    case $arg in
        --build-dir=*)
            BUILD_DIR="${arg#*=}"
            shift
            ;;
        --steps=*)
            FLOW_STEPS="${arg#*=}"
            shift
            ;;
        --help)
            echo "Usage: $0 [--build-dir=DIR] [--steps=STEPS]"
            echo "  --build-dir=DIR   Specify the build directory (default: build)"
            echo "  --steps=STEPS     Specify comma separated list of flow steps (synth, pnr, pack, prog)"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            # unknown option
            ;;
    esac
done


function synth() {
    out_json=$1
    rtl_srcs=("${@:2}")

    # if srcs are newr than out_json, run synthesis
    if [ ! -f $out_json ] || [ ${rtl_srcs[@]} -nt $out_json ]; then
        echo ">> Running synthesis..."
        yosys -p "read_verilog ${rtl_srcs[@]}; synth_gowin -top $TOPMODULE -json $out_json"
        if [ $? -ne 0 ]; then
            echo ">> Synthesis failed"
            exit 1
        fi
    else
        echo ">> Synthesis already done, skipping..."
    fi
}


function pnr() {
    out_json=$1
    synth_json=$2
    # if synth_json is newer than out_json, run pnr
    if [ ! -f $out_json ] || [ $synth_json -nt $out_json ]; then
        echo ">> Running Place and Route..."
        nextpnr-himbaechel --json $synth_json \
                           --write $out_json \
                           --device $DEVICE \
                           --vopt cst=$CONSTRAINT_FILE \
                           --vopt family=$FAMILY
        if [ $? -ne 0 ]; then
            echo ">> Place and Route failed"
            exit 1
        fi
    else
        echo ">> Place and Route already done, skipping..."
    fi
}


function pack() {
    out_fs=$1
    pnr_json=$2
    # if pnr_json is newer than out_fs, run pack
    if [ ! -f $out_fs ] || [ $pnr_json -nt $out_fs ]; then
        echo ">> Running packing..."
        gowin_pack -d $DEVICE -o $out_fs $pnr_json
        if [ $? -ne 0 ]; then
            echo ">> Packing failed"
            exit 1
        fi
    else
        echo ">> Packing already done, skipping..."
    fi
}


function prog() {
    out_fs=$1
    # if out_fs exists, run programming
    if [ -f $out_fs ]; then
        echo ">> Programming..."
        openFPGALoader -b $BOARD $out_fs
        if [ $? -ne 0 ]; then
            echo ">> Programming failed"
            exit 1
        fi
    else
        echo ">> Programming file not found, skipping..."
    fi
}


# Parse the flow steps (comma separated list)
IFS=',' read -ra items <<< "$FLOW_STEPS"
for item in "${items[@]}"; do
    echo ">> Running step: $item"
    case $item in
        synth)
            synth_json="$BUILD_DIR/${TOPMODULE}_syn.json"
            synth $synth_json $VERILOG_FILES
            ;;
        pnr)
            pnr_json="$BUILD_DIR/${TOPMODULE}_pnr.json"
            synth_json="$BUILD_DIR/${TOPMODULE}_syn.json"
            pnr $pnr_json $synth_json
            ;;
        pack)
            pack_fs="$BUILD_DIR/${TOPMODULE}_pack.fs"
            pnr_json="$BUILD_DIR/${TOPMODULE}_pnr.json"
            pack $pack_fs $pnr_json
            ;;
        prog)
            pack_fs="$BUILD_DIR/${TOPMODULE}_pack.fs"
            prog $pack_fs
            ;;
        *)
            echo "Unknown step: $item"
            exit 1
            ;;
    esac
done
