#ifndef I_INPUT_STREAM_H
#define I_INPUT_STREAM_H 0

//class EventObject;

class IInputStream {

    public:
        IInputStream() {}
        virtual ~IInputStream() {}

        virtual bool next(unsigned int nsteps=1, bool read=true) = 0;
        virtual bool prev(unsigned int nsteps=1, bool read=true) = 0;
        virtual bool first(bool read=true) = 0;
        virtual bool last(bool read=true) = 0;
        virtual bool reset() = 0;
       // virtual EventObject* get() = 0;
		virtual void* get() = 0;

};

inline bool IInputStream::next(unsigned int nsteps, bool read)
{
    return false;
}

inline bool IInputStream::prev(unsigned int nsteps, bool read)
{
    return false;
}

inline bool IInputStream::first(bool read)
{
    return false;
}

inline bool IInputStream::last(bool read)
{
    return false;
}

inline bool IInputStream::reset()
{
    return false;
}

#endif
