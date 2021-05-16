### Placeholder to store objects during communication

class Topic:
    def __init__(self, id, name, obj, converter, pub=None):
        self._id = id
        self._name = name
        self._obj = obj
        self._converter = converter
        self._pub = pub

    @property
    def id(self):
        return self._id

    @property
    def name(self):
        return self._name

    @property
    def obj(self):
        return self._obj

    @property
    def converter(self):
        return self._converter

    @property
    def pub(self):
        return self._pub