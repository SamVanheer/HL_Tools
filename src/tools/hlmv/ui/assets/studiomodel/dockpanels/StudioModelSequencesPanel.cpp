#include "engine/shared/activity.h"

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelEditEventsDialog.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelSequencesPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelSequencesPanel::StudioModelSequencesPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	connect(_ui.SequenceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnSequenceChanged);
	connect(_ui.LoopingModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnLoopingModeChanged);

	connect(_ui.BlendXSlider, &QSlider::valueChanged, this, &StudioModelSequencesPanel::OnBlendXSliderChanged);
	connect(_ui.BlendXSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnBlendXSpinnerChanged);
	connect(_ui.BlendYSlider, &QSlider::valueChanged, this, &StudioModelSequencesPanel::OnBlendYSliderChanged);
	connect(_ui.BlendYSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelSequencesPanel::OnBlendYSpinnerChanged);

	connect(_ui.EditEvents, &QPushButton::clicked, this, &StudioModelSequencesPanel::OnEditEvents);
	connect(_ui.EventsComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelSequencesPanel::OnEventChanged);

	connect(_ui.PlaySound, &QCheckBox::stateChanged, this, &StudioModelSequencesPanel::OnPlaySoundChanged);
	connect(_ui.PitchFramerateAmplitude, &QCheckBox::stateChanged, this, &StudioModelSequencesPanel::OnPitchFramerateAmplitudeChanged);

	auto sizePolicy = _ui.EventInfoWidget->sizePolicy();

	sizePolicy.setRetainSizeWhenHidden(true);

	_ui.EventInfoWidget->setSizePolicy(sizePolicy);

	_ui.EventInfoWidget->setVisible(false);

	auto model = _context->GetScene()->GetEntity()->GetModel();

	auto header = model->GetStudioHeader();

	QStringList sequences;

	sequences.reserve(header->numseq);

	for (int i = 0; i < header->numseq; ++i)
	{
		sequences.append(header->GetSequence(i)->label);
	}

	_ui.SequenceComboBox->addItems(sequences);
}

StudioModelSequencesPanel::~StudioModelSequencesPanel() = default;

void StudioModelSequencesPanel::UpdateBlendValue(int blender, BlendUpdateSource source, QSlider* slider, QDoubleSpinBox* spinner)
{
	if (source == BlendUpdateSource::Slider)
	{
		spinner->setValue(slider->value() / _blendsScales[blender]);
	}
	else
	{
		slider->setValue(spinner->value() * _blendsScales[blender]);
	}

	auto entity = _context->GetScene()->GetEntity();

	entity->SetBlending(blender, spinner->value());
}

void StudioModelSequencesPanel::OnSequenceChanged(int index)
{
	auto entity = _context->GetScene()->GetEntity();

	entity->SetSequence(index);

	auto sequence = entity->GetModel()->GetStudioHeader()->GetSequence(index);

	const auto durationInSeconds = sequence->numframes / sequence->fps;

	_ui.SequenceLabel->setText(QString::number(index));
	_ui.FrameCountLabel->setText(QString::number(sequence->numframes));
	_ui.FPSLabel->setText(QString::number(sequence->fps, 'g', 2));
	_ui.DurationLabel->setText(QString::number(durationInSeconds, 'g', 2));

	_ui.EventCountLabel->setText(QString::number(sequence->numevents));
	_ui.IsLoopingLabel->setText((sequence->flags & STUDIO_LOOPING) ? "Yes" : "No");
	_ui.BlendCountLabel->setText(QString::number(sequence->numblends));
	_ui.ActivityWeightLabel->setText(QString::number(sequence->actweight));

	QString activityName{"Unknown"};

	if (sequence->activity >= ACT_IDLE && sequence->activity <= ACT_FLINCH_RIGHTLEG)
	{
		activityName = activity_map[sequence->activity - 1].name;
	}
	else if (sequence->activity == ACT_RESET)
	{
		activityName = "None";
	}

	_ui.ActivityNameLabel->setText(QString{"%1 (%2)"}.arg(activityName).arg(sequence->activity));

	const float initialBlendValue = 0.f;

	entity->SetBlending(0, initialBlendValue);
	entity->SetBlending(1, initialBlendValue);

	QSlider* const sliders[] =
	{
		_ui.BlendXSlider,
		_ui.BlendYSlider
	};

	QDoubleSpinBox* const spinners[] =
	{
		_ui.BlendXSpinner,
		_ui.BlendYSpinner
	};

	for (int blender = 0; blender < 2; ++blender)
	{
		const auto hasBlender = sequence->blendtype[blender] != 0;

		const auto slider = sliders[blender];

		const auto spinner = spinners[blender];

		if (hasBlender)
		{
			float start, end;

			//Swap values if the range is inverted
			if (sequence->blendend[blender] < sequence->blendstart[blender])
			{
				start = sequence->blendend[blender];
				end = sequence->blendstart[blender];
			}
			else
			{
				start = sequence->blendstart[blender];
				end = sequence->blendend[blender];
			}

			//Should probably scale as needed so the range is sufficiently large
			//This prevents ranges that cover less than a whole integer from not doing anything
			if ((end - start) < 1.0f)
			{
				_blendsScales[blender] = 100.0f;
			}
			else
			{
				_blendsScales[blender] = 1.0f;
			}

			//Using this avoids the lossy round trip, which makes the value more accurate
			const auto value = initialBlendValue;

			slider->setRange(static_cast<int>(start * _blendsScales[blender]), static_cast<int>(end * _blendsScales[blender]));
			slider->setValue(value * _blendsScales[blender]);

			spinner->setRange(start, end);
			spinner->setValue(value);
		}
		else
		{
			//Make the slider be nice and centered
			slider->setRange(-1, 1);
			slider->setValue(0);

			spinner->setRange(-1, 1);
			spinner->setValue(0);
		}

		slider->setEnabled(hasBlender);
		spinner->setEnabled(hasBlender);
	}

	_ui.EventsComboBox->clear();

	const bool hasEvents = sequence->numevents > 0;

	if (hasEvents)
	{
		QStringList events;

		events.reserve(sequence->numevents);

		for (int i = 0; i < sequence->numevents; ++i)
		{
			events.append(QString("Event %1").arg(i + 1));
		}

		_ui.EventsComboBox->addItems(events);
	}

	_ui.EventsComboBox->setEnabled(hasEvents);
}

void StudioModelSequencesPanel::OnLoopingModeChanged(int index)
{
	_context->GetScene()->GetEntity()->SetLoopingMode(static_cast<StudioLoopingMode>(index));
}

void StudioModelSequencesPanel::OnBlendXSliderChanged()
{
	UpdateBlendValue(0, BlendUpdateSource::Slider, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void StudioModelSequencesPanel::OnBlendXSpinnerChanged()
{
	UpdateBlendValue(0, BlendUpdateSource::Spinner, _ui.BlendXSlider, _ui.BlendXSpinner);
}

void StudioModelSequencesPanel::OnBlendYSliderChanged()
{
	UpdateBlendValue(1, BlendUpdateSource::Slider, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void StudioModelSequencesPanel::OnBlendYSpinnerChanged()
{
	UpdateBlendValue(1, BlendUpdateSource::Spinner, _ui.BlendYSlider, _ui.BlendYSpinner);
}

void StudioModelSequencesPanel::OnEditEvents()
{
	StudioModelEditEventsDialog dialog(_context, _ui.SequenceComboBox->currentIndex(), this);

	if (QDialog::DialogCode::Accepted == dialog.exec())
	{
		//Refresh event info
		OnEventChanged(_ui.EventsComboBox->currentIndex());
	}
}

void StudioModelSequencesPanel::OnEventChanged(int index)
{
	const bool hasEvent = index != -1;

	if (hasEvent)
	{
		auto entity = _context->GetScene()->GetEntity();

		auto model = entity->GetModel();

		auto sequence = model->GetStudioHeader()->GetSequence(entity->GetSequence());

		auto event = reinterpret_cast<const mstudioevent_t*>(model->GetStudioHeader()->GetData() + sequence->eventindex) + index;

		_ui.EventFrameIndexLabel->setText(QString::number(event->frame));
		_ui.EventIdLabel->setText(QString::number(event->event));
		_ui.EventOptionsLabel->setText(event->options);
		_ui.EventTypeLabel->setText(QString::number(event->type));
	}

	_ui.EventInfoWidget->setVisible(hasEvent);
}

void StudioModelSequencesPanel::OnPlaySoundChanged()
{
	_context->GetScene()->GetEntity()->PlaySound = _ui.PlaySound->isChecked();
}

void StudioModelSequencesPanel::OnPitchFramerateAmplitudeChanged()
{
	_context->GetScene()->GetEntity()->PitchFramerateAmplitude = _ui.PitchFramerateAmplitude->isChecked();
}
}
