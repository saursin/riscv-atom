#include "vuart.hpp"
#include "except.hpp"

#include <string>
#include <cstring>
#include <csignal>
#include <iostream>
#include <fcntl.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

// link to the pseudo-terminal: removed at exit, also on SIGTERM and SIGHUP
static char pty_link[4096];

static void remove_link()
{
    if(pty_link[0])
        unlink(pty_link);
    pty_link[0] = 0;
}

static void remove_link_on_signal(int sig)
{
    remove_link();
    signal(sig, SIG_DFL);
    raise(sig);
}

// Helper functions
inline void Vuart::_openPort()
{
    if(pty_fd < 0)
        port->Open();
    isopen = true;
}


inline void Vuart::_closePort()
{
    if(pty_fd < 0)
        port->Close();
    isopen = false;
}


Vuart::Vuart(std::string portname, int baud, bool wait)
{
    // set baud rate
    setbaud(baud);

    if(portname == "pty")
    {
        pty_fd = posix_openpt(O_RDWR | O_NOCTTY);
        if(pty_fd < 0 || grantpt(pty_fd) || unlockpt(pty_fd))
            throw Atomsim_exception("could not create a pseudo-terminal");
        std::string name = ptsname(pty_fd);

        // open and close it once: until a terminal opens it again, it then reports a hang-up
        close(open(name.c_str(), O_RDWR | O_NOCTTY));

        // raw: bytes pass unchanged, and are not echoed back to the simulation
        struct termios t;
        ioctl(pty_fd, TCGETS, &t);
        t.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        t.c_oflag &= ~OPOST;
        t.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        t.c_cflag = (t.c_cflag & ~(CSIZE | PARENB)) | CS8;
        ioctl(pty_fd, TCSETS, &t);
        fcntl(pty_fd, F_SETFL, O_NONBLOCK);

        // $RVATOM/sim.tty, where atomterm looks when given no port
        const char *rvatom = getenv("RVATOM");
        if(!rvatom)
            throw Atomsim_exception("cant find $RVATOM env variable");
        std::string link = std::string(rvatom) + "/sim.tty";
        struct stat st;
        if(lstat(link.c_str(), &st) == 0 && S_ISLNK(st.st_mode))
            unlink(link.c_str());       // left over from an earlier run
        if(symlink(name.c_str(), link.c_str()))
            throw Atomsim_exception("could not create link " + link + " (a file with that name exists?)");
        strncpy(pty_link, link.c_str(), sizeof(pty_link) - 1);
        atexit(remove_link);
        signal(SIGTERM, remove_link_on_signal);
        signal(SIGHUP, remove_link_on_signal);

        std::cout << "VUART on " << link << " (" << name << ")" << std::endl;
        if(wait)
        {
            std::cout << "waiting for a terminal: run atomterm" << std::endl;
            struct pollfd p = {pty_fd, POLLIN, 0};
            while(poll(&p, 1, 0) >= 0 && (p.revents & POLLHUP))
                if(usleep(100000) < 0)      // interrupted (Ctrl-C)
                    throw Atomsim_exception("stopped while waiting for a terminal");
        }
        isopen = true;
        return;
    }

    // create a new port object
    port = new mn::CppLinuxSerial::SerialPort(portname, port_baudrate);

    /* Set timeout to 0: i.e. Non blocking Mode
        This (paired with 1 byte recieve buffer of the library) enables the latest pressed key 
        on the keyboard to be returned while calling recieve().
    */
    port->SetTimeout(0);

    // Open port
    _openPort();
}


Vuart::~Vuart()
{
    // close port
    _closePort();

    // destroy port object
    delete port;

    if(pty_fd >= 0)
    {
        close(pty_fd);
        remove_link();
    }
}


void Vuart::setbaud(unsigned int baud)
{
    switch(baud)
    {
        case 9600:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_9600; break;
        case 19200:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_19200; break;
        case 115200:
            port_baudrate=mn::CppLinuxSerial::BaudRate::B_115200; break;
        
        default:
            throw "Invalid baud rate";
    }

    // if already opened (a pseudo-terminal has no baud rate)
    if(isopen && pty_fd < 0)
    {
        // Close port, change baudrate, and reopen
        _closePort();

        port->SetBaudRate(port_baudrate);
        
        _openPort();
    }
}



unsigned int Vuart::getbaud()
{
    switch(port_baudrate)
    {
        case mn::CppLinuxSerial::BaudRate::B_9600:
            return 9600;
        case mn::CppLinuxSerial::BaudRate::B_19200:
            return 19200;
        case mn::CppLinuxSerial::BaudRate::B_115200:
            return 115200;
        
        default: 
            return -1;  // unknown baud
    }
}


bool Vuart::isOpen()
{
    return isopen;
}


void Vuart::send(char c)
{
    if(pty_fd >= 0)
    {
        ssize_t n = write(pty_fd, &c, 1);   // fails (dropped) while no terminal is connected
        (void)n;
        return;
    }
    std::vector<uint8_t> vec;
    vec.push_back((uint8_t) c);
    port->WriteBinary(vec);
}


int Vuart::recieve()
{        
    if(pty_fd >= 0)
    {
        unsigned char c;
        return read(pty_fd, &c, 1) == 1 ? c : -1;
    }
    std::vector<uint8_t> vec;
    port->ReadBinary(vec);

    if(vec.size()==0)
        return (int) -1;
    else
        return (int) vec[0];
}


void Vuart::clean_recieve_buffer()
{
    char c;
    do
    {
        c = recieve();  // dump garbage
    } while(c != -1);
}

