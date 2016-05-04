#ifndef SerialCommunication_h
#define SerialCommunication_h

typedef uint8_t (*ReceptionOpe)(char);

class SerialCommunication
{
public:
  void Initialization(void);
  void SetReceptOpe(ReceptionOpe* pRcvOpeSet);

private:
};

#endif  // SerialCommunication_h
