#include <Python.h>
#include <string>
#include <vector>

// Inclure les headers de mini-tor
#include "../mini/logger.h"
#include "../mini/tor/circuit.h"
#include "../mini/tor/consensus.h"
#include "../mini/tor/onion_router.h"
#include "../mini/tor/tor_socket.h"
#include "../mini/tor/tor_stream.h"
#include "../mini/tor/hidden_service.h"

// Structure pour stocker les données de contexte
typedef struct {
    PyObject_HEAD
    mini::tor::consensus* consensus;
    mini::tor::circuit* circuit;
    mini::tor::tor_stream* stream;
} MiniTorObject;

// Fonction de destruction
static void
MiniTor_dealloc(MiniTorObject* self)
{
    if (self->stream) {
        delete self->stream;
        self->stream = nullptr;
    }
    
    if (self->circuit) {
        delete self->circuit;
        self->circuit = nullptr;
    }
    
    if (self->consensus) {
        delete self->consensus;
        self->consensus = nullptr;
    }
    
    Py_TYPE(self)->tp_free((PyObject*)self);
}

// Fonction d'initialisation
static PyObject*
MiniTor_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    MiniTorObject* self;
    self = (MiniTorObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->consensus = nullptr;
        self->circuit = nullptr;
        self->stream = nullptr;
    }
    return (PyObject*)self;
}

// Fonction d'initialisation avec paramètres
static int
MiniTor_init(MiniTorObject* self, PyObject* args, PyObject* kwds)
{
    // Initialiser le consensus
    self->consensus = new mini::tor::consensus();
    if (!self->consensus->initialize()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to initialize Tor consensus");
        return -1;
    }
    
    return 0;
}

// Méthode pour se connecter à une adresse onion
static PyObject*
MiniTor_connect(MiniTorObject* self, PyObject* args)
{
    const char* onion_address;
    int port = 80;  // Port par défaut
    
    // Analyser les arguments
    if (!PyArg_ParseTuple(args, "s|i", &onion_address, &port)) {
        return NULL;
    }
    
    // Créer un circuit si nécessaire
    if (!self->circuit) {
        self->circuit = new mini::tor::circuit(*self->consensus);
        if (!self->circuit->initialize()) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to initialize Tor circuit");
            return NULL;
        }
    }
    
    // Créer un stream
    if (self->stream) {
        delete self->stream;
        self->stream = nullptr;
    }
    
    self->stream = new mini::tor::tor_stream(*self->circuit);
    
    // Se connecter à l'adresse onion
    mini::string onion_str(onion_address);
    if (!self->stream->connect(onion_str, port)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to connect to onion address");
        return NULL;
    }
    
    Py_RETURN_NONE;
}

// Méthode pour envoyer des données
static PyObject*
MiniTor_send(MiniTorObject* self, PyObject* args)
{
    const char* data;
    Py_ssize_t data_len;
    
    // Analyser les arguments
    if (!PyArg_ParseTuple(args, "s#", &data, &data_len)) {
        return NULL;
    }
    
    // Vérifier que le stream est initialisé
    if (!self->stream) {
        PyErr_SetString(PyExc_RuntimeError, "Not connected to any onion address");
        return NULL;
    }
    
    // Envoyer les données
    size_t bytes_sent = self->stream->send((const uint8_t*)data, data_len);
    
    return PyLong_FromSize_t(bytes_sent);
}

// Méthode pour recevoir des données
static PyObject*
MiniTor_receive(MiniTorObject* self, PyObject* args)
{
    size_t max_size = 4096;  // Taille par défaut
    
    // Analyser les arguments
    if (!PyArg_ParseTuple(args, "|n", &max_size)) {
        return NULL;
    }
    
    // Vérifier que le stream est initialisé
    if (!self->stream) {
        PyErr_SetString(PyExc_RuntimeError, "Not connected to any onion address");
        return NULL;
    }
    
    // Allouer un buffer pour les données reçues
    std::vector<uint8_t> buffer(max_size);
    
    // Recevoir les données
    size_t bytes_received = self->stream->receive(buffer.data(), buffer.size());
    
    // Créer un objet bytes Python avec les données reçues
    return PyBytes_FromStringAndSize((const char*)buffer.data(), bytes_received);
}

// Méthode pour fermer la connexion
static PyObject*
MiniTor_close(MiniTorObject* self, PyObject* Py_UNUSED(ignored))
{
    if (self->stream) {
        delete self->stream;
        self->stream = nullptr;
    }
    
    if (self->circuit) {
        delete self->circuit;
        self->circuit = nullptr;
    }
    
    Py_RETURN_NONE;
}

// Méthode pour obtenir la liste des routeurs
static PyObject*
MiniTor_get_routers(MiniTorObject* self, PyObject* Py_UNUSED(ignored))
{
    if (!self->consensus) {
        PyErr_SetString(PyExc_RuntimeError, "Consensus not initialized");
        return NULL;
    }
    
    // Obtenir la liste des routeurs
    auto routers = self->consensus->get_valid_routers({}, {}, {}, {});
    
    // Créer une liste Python
    PyObject* router_list = PyList_New(routers.get_size());
    if (!router_list) {
        return NULL;
    }
    
    // Remplir la liste avec les informations des routeurs
    for (size_t i = 0; i < routers.get_size(); i++) {
        auto router = routers[i];
        if (!router) {
            continue;
        }
        
        PyObject* router_dict = PyDict_New();
        if (!router_dict) {
            Py_DECREF(router_list);
            return NULL;
        }
        
        // Ajouter les informations du routeur au dictionnaire
        PyDict_SetItemString(router_dict, "name", PyUnicode_FromString(router->get_name().get_buffer()));
        PyDict_SetItemString(router_dict, "identity", PyUnicode_FromString(router->get_identity().to_base64().get_buffer()));
        PyDict_SetItemString(router_dict, "ip", PyUnicode_FromString(router->get_ip_address().to_string().get_buffer()));
        PyDict_SetItemString(router_dict, "or_port", PyLong_FromLong(router->get_or_port()));
        PyDict_SetItemString(router_dict, "dir_port", PyLong_FromLong(router->get_dir_port()));
        
        // Ajouter le dictionnaire à la liste
        PyList_SetItem(router_list, i, router_dict);
    }
    
    return router_list;
}

// Définition des méthodes
static PyMethodDef MiniTor_methods[] = {
    {"connect", (PyCFunction)MiniTor_connect, METH_VARARGS, "Connect to an onion address"},
    {"send", (PyCFunction)MiniTor_send, METH_VARARGS, "Send data to the connected onion address"},
    {"receive", (PyCFunction)MiniTor_receive, METH_VARARGS, "Receive data from the connected onion address"},
    {"close", (PyCFunction)MiniTor_close, METH_NOARGS, "Close the connection"},
    {"get_routers", (PyCFunction)MiniTor_get_routers, METH_NOARGS, "Get the list of Tor routers"},
    {NULL}  /* Sentinel */
};

// Définition du type
static PyTypeObject MiniTorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mini_tor.MiniTor",
    .tp_doc = "MiniTor object",
    .tp_basicsize = sizeof(MiniTorObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = MiniTor_new,
    .tp_init = (initproc)MiniTor_init,
    .tp_dealloc = (destructor)MiniTor_dealloc,
    .tp_methods = MiniTor_methods,
};

// Définition du module
static PyModuleDef mini_tor_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "mini_tor",
    .m_doc = "Python bindings for mini-tor library",
    .m_size = -1,
};

// Fonction d'initialisation du module
PyMODINIT_FUNC
PyInit_mini_tor(void)
{
    PyObject* m;
    
    // Finaliser le type
    if (PyType_Ready(&MiniTorType) < 0)
        return NULL;
    
    // Créer le module
    m = PyModule_Create(&mini_tor_module);
    if (m == NULL)
        return NULL;
    
    // Ajouter le type au module
    Py_INCREF(&MiniTorType);
    if (PyModule_AddObject(m, "MiniTor", (PyObject*)&MiniTorType) < 0) {
        Py_DECREF(&MiniTorType);
        Py_DECREF(m);
        return NULL;
    }
    
    return m;
}