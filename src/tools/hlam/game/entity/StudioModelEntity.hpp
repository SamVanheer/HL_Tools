#pragma once

#include <vector>

#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "engine/shared/studiomodel/StudioModel.hpp"

#include "game/AnimEvent.hpp"
#include "game/Events.hpp"

#include "game/entity/BaseAnimating.hpp"

enum class StudioLoopingMode
{
	AlwaysLoop = 0,
	NeverLoop,
	UseSequenceSetting
};

/**
*	Studio model entity.
*/
class StudioModelEntity : public BaseAnimating
{
public:
	DECLARE_CLASS(StudioModelEntity, BaseAnimating);

public:
	virtual bool Spawn() override;

	virtual void Draw(renderer::DrawFlags flags) override;

	studiomdl::ModelRenderInfo GetRenderInfo() const;

	/**
	*	Advances the frame. If dt is 0, advances to current time, otherwise, advances by the given amount of time.
	*	TODO: clamp dt to positive?
	*	@param deltaTime Delta time.
	*	@param maximum Maximum amount of time to advance by. If -1, no limit.
	*	@return Delta time that was used to advance the frame. Can be 0.
	*/
	float	AdvanceFrame(float deltaTime = 0.0f, const float maximum = -1.f);

	/**
	*	Gets an animation event for the current sequence for the given time range.
	*	@param event Output. Event data.
	*	@param start Start of the range of frames to check.
	*	@param end End of the range of frames to check.
	*	@param index Event index to start checking at.
	*	@param allowClientEvents Whether to process client events or not.
	*	@return Next event index to use as the index parameter. If 0, no more events are left.
	*/
	int		GetAnimationEvent(AnimEvent& event, float start, float end, int index, const bool allowClientEvents);

	/**
	*	Dispatches events for the current sequence and frame. This will dispatch events between the frame number during last call to DispatchAnimEvents and the current frame.
	*	@param allowClientEvents Whether to process client events or not.
	*/
	void	DispatchAnimEvents(const bool allowClientEvents);

	/**
	*	Method to handle animation events. Override to handle events.
	*/
	virtual void HandleAnimEvent(const AnimEvent& event);

public:
	/**
	*	@brief Sets the frame for this model.
	*	@param frame Frame index.
	*/
	void SetFrame(float frame);

private:
	studiomdl::StudioModel* _model = nullptr;

	int		_sequence = 0;				// sequence index
	int		_bodygroup = 0;				// bodypart selection	
	int		_skin = 0;				// skin group selection
	byte	_controller[STUDIO_MAX_CONTROLLERS] = {0, 0, 0, 0};	// bone controllers
	float _controllerValues[STUDIO_MAX_CONTROLLERS] = {};
	byte	_mouth = 0;				// mouth position
	byte	_blending[STUDIO_MAX_BLENDERS] = {0, 0};			// animation blending

	float	_lastEventCheck = 0;				//Last time we checked for animation events.
	float	_animTime = 0;				//Time when the frame was set.

	StudioLoopingMode _loopingMode = StudioLoopingMode::AlwaysLoop;

public:
	/**
	*	Gets the model.
	*/
	studiomdl::StudioModel* GetModel() const { return _model; }

	/**
	*	Sets the model.
	*/
	void SetModel(studiomdl::StudioModel* model);

	/**
	*	Gets the number of frames that the current sequence has.
	*/
	int GetNumFrames() const;

	/**
	*	Gets the current sequence index.
	*/
	int GetSequence() const { return _sequence; }

	/**
	*	Sets the current sequence.
	*	@param sequence Sequence to use.
	*/
	void SetSequence(const int sequence);

	/**
	*	Gets info from the current sequence.
	*	@param frameRate The sequence's frame rate.
	*	@param groundSpeed How fast the entity should move on the ground during this sequence.
	*/
	void GetSequenceInfo(float& frameRate, float& groundSpeed) const;

	/**
	*	Gets the bodygroup.
	*/
	int GetBodygroup() const { return _bodygroup; }

	int GetBodyValueForGroup(int group) const;

	/**
	*	Sets the value for the given bodygroup.
	*	@param bodygroup Bodygroup to set.
	*	@param value Value to set.
	*/
	void SetBodygroup(const int bodygroup, const int value);

	/**
	*	Gets the current skin.
	*/
	int GetSkin() const { return _skin; }

	/**
	*	Sets the current skin.
	*	@param skin Skin to use.
	*/
	void SetSkin(const int skin);

	/**
	*	Gets the given controller by index. This is the stored value, not the computed value.
	*	@param controller Controller to get.
	*/
	byte GetControllerByIndex(const int controller) const;

	/**
	*	Gets the given controller value. The value is computed for the associated bone controller.
	*	@param controller Controller value to get.
	*/
	float GetControllerValue(const int controller) const;

	/**
	*	Sets the controller value. The value is processed into a value that is in the range [0, 255].
	*	@param controller Controller to set.
	*	@param value Value to set.
	*/
	void SetController(const int controller, float value);

	/**
	*	Gets the mouth controller. This is the stored value, not the computed value.
	*/
	byte GetMouth() const { return _mouth; }

	/**
	*	Sets the mouth controller value. The value is processed into a value that is in the range [0, 255]
	*	@param value Value to set.
	*/
	void SetMouth(float value);

	/**
	*	Gets the given blender by index. This is the stored value, not the computed value.
	*	@param blender Blender to get.
	*/
	byte GetBlendingByIndex(const int blender) const;

	/**
	*	Gets the given blender value. The value is computed for the associated blender.
	*	@param blender Blender value to get.
	*/
	float GetBlendingValue(const int blender) const;

	/**
	*	Sets the given blender's value.
	*	@param blender Blender to set.
	*	@param value Value to set.
	*/
	void SetBlending(const int blender, float value);

	/**
	*	Gets the last event check. This is the end of the range used to check for animation events the last time DispatchAnimEvents was called.
	*/
	float GetLastEventCheck() const { return _lastEventCheck; }

	/**
	*	Gets the last time this entity advanced its frame.
	*/
	float GetAnimTime() const { return _animTime; }

	StudioLoopingMode GetLoopingMode() const { return _loopingMode; }

	void SetLoopingMode(StudioLoopingMode value)
	{
		_loopingMode = value;
	}

	/**
	*	Extracts the bounding box from the current sequence.
	*/
	void ExtractBbox(glm::vec3& mins, glm::vec3& maxs) const;

	/**
	*	Gets a model by body part.
	*/
	mstudiomodel_t* GetModelByBodyPart(const int bodyPart) const;

	/**
	*	Computes a list of meshes that use the given texture.
	*/
	std::vector<const mstudiomesh_t*> ComputeMeshList(const int texture) const;
};
