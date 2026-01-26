// Example of a data structure a mod might interact with
#include "data.h"
#include <map>
#include <string>

struct ModSpecificData {
    std::map<std::string, float> modFloats;
    std::map<std::string, int> modInts;
    // ... other types
};

class IMod {
  public:
    virtual ~IMod() = default;
    virtual const char *getName() const = 0;
    virtual void onInit(PhysicsComponent *phys, RenderComponent *render) = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onEntityCreated() = 0;
    virtual void onEntityDestroyed() = 0;
};

class ModManager {
  private:
    std::vector<IMod *> loadedMods;

  public:
    void loadMods(const std::string &directory, PhysicsComponent *phys,
                  RenderComponent *render) {}

    void update(float deltaTime) {
        for (IMod *mod : loadedMods) {
            mod->onUpdate(deltaTime);
        }
    }

    void render() {
        for (IMod *mod : loadedMods) {
            mod->onRender();
        }
    }
};
