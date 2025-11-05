#!/bin/bash
# =====================================================================
# UF HiPerGator ADIOS2 + CAESAR environment setup script
# Author: Ethan Klasky
# =====================================================================

# --- Step 1: Clean out any old ADIOS2 or CAESAR references from PATHs
clean_path() {
    echo "$1" | tr ':' '\n' | \
        grep -v '/home/adios/local/adios-install' | \
        grep -v '/opt/adios2' | \
        grep -v '/lustre/blue/ranka/eklasky/ADIOS2/install' | \
        grep -v '/lustre/blue/ranka/eklasky/CAESAR_C/install' | \
        paste -sd:
}

export PATH=$(clean_path "$PATH")
export LD_LIBRARY_PATH=$(clean_path "$LD_LIBRARY_PATH")
export LIBRARY_PATH=$(clean_path "$LIBRARY_PATH")
export CPATH=$(clean_path "$CPATH")
export PKG_CONFIG_PATH=$(clean_path "$PKG_CONFIG_PATH")
export MANPATH=$(clean_path "$MANPATH")

# --- Step 2: Set new installation prefixes
export ADIOS2_DIR=/lustre/blue/ranka/eklasky/ADIOS2/install
export CAESAR_DIR=/lustre/blue/ranka/eklasky/CAESAR_C/install

# --- Step 3: Add to environment paths
export PATH=$ADIOS2_DIR/bin:$PATH
export LD_LIBRARY_PATH=$ADIOS2_DIR/lib64:$CAESAR_DIR/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$ADIOS2_DIR/lib64:$CAESAR_DIR/lib:$LIBRARY_PATH
export CPATH=$ADIOS2_DIR/include:$CAESAR_DIR/include:$CPATH
export PKG_CONFIG_PATH=$ADIOS2_DIR/lib64/pkgconfig:$CAESAR_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
export MANPATH=$ADIOS2_DIR/share/man:$MANPATH

# --- Step 4: Print summary
echo "============================================================"
echo "Environment set for ADIOS2 + CAESAR"
echo "------------------------------------------------------------"
echo "ADIOS2_DIR       = $ADIOS2_DIR"
echo "CAESAR_DIR       = $CAESAR_DIR"
echo
echo "PATH additions:"
echo "  $(echo $PATH | tr ':' '\n' | grep -E 'ADIOS2|CAESAR')"
echo
echo "LD_LIBRARY_PATH  = $LD_LIBRARY_PATH"
echo "LIBRARY_PATH     = $LIBRARY_PATH"
echo "CPATH            = $CPATH"
echo "PKG_CONFIG_PATH  = $PKG_CONFIG_PATH"
echo "MANPATH          = $MANPATH"
echo "============================================================"

