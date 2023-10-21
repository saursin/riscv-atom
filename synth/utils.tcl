
# Parse a verilog listfile (.F)
proc parse_listfile {listfile} {
    set listfile_dir [file dirname $listfile]
    set verilog_files {}
    set include_dirs {}

    # Open and read the file list file
    set file [open $listfile r]
    while {[gets $file line] != -1} {
        # Remove leading and trailing whitespace from the line
        set line [string trim $line]

        # Skip empty lines
        if { [string length $line] == 0 } {
            continue
        }

        # Check if the line is a comment (starts with "//") & skip
        if { [regexp {\/\/.*$} $line] } {
            continue
        }

        # Check if the line has include statement
        if { [regexp {\+incdir\+.*$} $line] } {
            lappend include_dirs [file normalize $listfile_dir/[string range $line 8 end]]
            continue
        }

        # Add the file name to the array
        lappend verilog_files [file normalize $listfile_dir/$line]
    }
    close $file

    # Return both arrays as a list
    return [list $verilog_files $include_dirs]
}
