#pragma once
class InstanceAsset
{
public:
    InstanceAsset(int reg_id);
    int get_id();
private:
    int registry_id;
};
