#include "Branch.h"

class OgreBranch : public branch {

public:
    OgreBranch(int level, V3 basePosition);
    virtual branch *CreateNextBranch(int level, V3 basePosition);
    virtual void fillBuffers();

private:
    OgreBranch(){};
};