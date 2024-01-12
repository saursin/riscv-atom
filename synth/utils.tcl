
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

# Print elapsed time
proc print_elapsed_time {time_start time_end} {
    # Calculate elapsed time in seconds
    set time_elapsed [expr {$time_end - $time_start}]
    set time_elapsed_days [expr {$time_elapsed / (24 * 3600)}]
    set time_elapsed [expr {$time_elapsed % (24 * 3600)}]
    set time_elapsed_hours [expr {$time_elapsed / 3600}]
    set time_elapsed [expr {$time_elapsed % 3600}]
    set time_elapsed_minutes [expr {$time_elapsed / 60}]
    set time_elapsed_seconds [expr {$time_elapsed % 60}]
    set formatted_time [format "%02d d %02d h %02d m %02d s" $time_elapsed_days $time_elapsed_hours $time_elapsed_minutes $time_elapsed_seconds]
    puts "Time Elapsed: $formatted_time"
}