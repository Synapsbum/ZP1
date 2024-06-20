#pragma once

#include "GameCommon.h"
//#include "Particle_Int.h"

class obj_GasArea : public MeshGameObject
{
	DECLARE_CLASS(obj_GasArea, MeshGameObject)
	
public:
	float		useRadius;
	float		GAreaX;
	float		GAreaY;
	float		GAreaZ;
		
	static std::vector<obj_GasArea*> LoadedGasArea;
public:
	obj_GasArea();	
	virtual ~obj_GasArea();
	

	virtual	BOOL		Load(const char *name);

	virtual	BOOL		OnCreate();
	virtual	BOOL		OnDestroy();
	class obj_ParticleSystem* m_gasAreaParticle;	
	
	bool				canSpawnParticle;

	virtual	BOOL		Update();
 #ifndef FINAL_BUILD
 	virtual	float		DrawPropertyEditor(float scrx, float scry, float scrw, float scrh, const AClass* startClass, const GameObjects& selected) OVERRIDE;
 #endif
	virtual	void		AppendRenderables(RenderArray (& render_arrays  )[ rsCount ], const r3dCamera& Cam);
	virtual void		WriteSerializedData(pugi::xml_node& node);
	virtual	void		ReadSerializedData(pugi::xml_node& node);
};
