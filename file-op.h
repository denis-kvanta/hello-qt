int SpiOpenPort ();

int SpiRead (const unsigned char *AddrBuf, unsigned char *RxData, int Length);

int SpiWrite (const unsigned char *AddrBuf, const unsigned char *TxData, int Length);

int SpiClosePort ();