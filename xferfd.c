#include <sys/types.h>
#include <sys/socket.h>

#include <Python.h>

static PyObject *
xferfd_sendfd(PyObject *self, PyObject *args)
{
    PyObject *pyobj_conn, *pyobj_fd;
    int conn, fd, res;
    char dummy_char;
    char buf[CMSG_SPACE(sizeof(int))];
    struct msghdr msg = {0};
    struct iovec dummy_iov;
    struct cmsghdr *cmsg;

    if (!PyArg_ParseTuple(args, "OO", &pyobj_conn, &pyobj_fd))
        return NULL;

    conn = PyObject_AsFileDescriptor(pyobj_conn);
    if (conn == -1)
        return NULL;
    fd = PyObject_AsFileDescriptor(pyobj_fd);
    if (fd == -1)
        return NULL;

    dummy_iov.iov_base = &dummy_char;
    dummy_iov.iov_len = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    msg.msg_iov = &dummy_iov;
    msg.msg_iovlen = 1;
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    msg.msg_controllen = cmsg->cmsg_len;
    *(int*)CMSG_DATA(cmsg) = fd;

    Py_BEGIN_ALLOW_THREADS
    res = sendmsg(conn, &msg, 0);
    Py_END_ALLOW_THREADS

    if (res < 0)
        return PyErr_SetFromErrno(PyExc_OSError);
    Py_RETURN_NONE;
}

PyDoc_STRVAR(xferfd_sendfd__doc__,
"sendfd(socket, fd)\n"
"\n"
"Send a file descriptor over a socket.\n\n"
"Arguments:\n"
"socket -- the socket to send the fd too\n"
"fd -- the file descriptor to send");

static PyObject *
xferfd_recvfd(PyObject *self, PyObject *args)
{
    PyObject *pyobj_conn;
    int conn, fd, res;
    char dummy_char;
    char buf[CMSG_SPACE(sizeof(int))];
    struct msghdr msg = {0};
    struct iovec dummy_iov;
    struct cmsghdr *cmsg;

    if (!PyArg_ParseTuple(args, "O", &pyobj_conn))
        return NULL;

    conn = PyObject_AsFileDescriptor(pyobj_conn);
    if (conn == -1)
        return NULL;

    dummy_iov.iov_base = &dummy_char;
    dummy_iov.iov_len = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);
    msg.msg_iov = &dummy_iov;
    msg.msg_iovlen = 1;
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    msg.msg_controllen = cmsg->cmsg_len;

    Py_BEGIN_ALLOW_THREADS
    res = recvmsg(conn, &msg, 0);
    Py_END_ALLOW_THREADS

    if (res < 0)
        return PyErr_SetFromErrno(PyExc_OSError);

    fd = *(int*)CMSG_DATA(cmsg);
    return Py_BuildValue("i", fd);
}

PyDoc_STRVAR(xferfd_recvfd__doc__,
"recvfd(socket) -> int\n"
"\n"
"Returns a file descriptor from a socket.\n\n"
"Arguments:\n"
"socket -- the socket to receive the fd from");

static PyMethodDef xferfdMethods[] = {
    {"sendfd", xferfd_sendfd, METH_VARARGS, xferfd_sendfd__doc__},
    {"recvfd", xferfd_recvfd, METH_VARARGS, xferfd_recvfd__doc__},
    {NULL, NULL, 0, NULL} // Sentinel
};

PyMODINIT_FUNC
initxferfd(void)
{
    (void) Py_InitModule("xferfd", xferfdMethods);
}
