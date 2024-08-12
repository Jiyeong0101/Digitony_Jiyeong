#include "Widget/CSoundWidget.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Sound/SoundClass.h"

void UCSoundWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // üũ�ڽ��� �����̴��� �ʱ� ���� ���� �� �̺�Ʈ ���ε�
    if (SoundToggleCheckBox)
    {
        SoundToggleCheckBox->SetIsChecked(bIsSoundEnabled);
        SoundToggleCheckBox->OnCheckStateChanged.AddDynamic(this, &UCSoundWidget::OnSoundToggleChanged);
    }

    if (VolumeSlider)
    {
        VolumeSlider->SetValue(InitialVolume);
        VolumeSlider->OnValueChanged.AddDynamic(this, &UCSoundWidget::OnVolumeSliderChanged);
    }

    // �ʱ� ���¿� ���� ���� Ŭ������ ���� ����
    if (SoundClass)
    {
        SoundClass->Properties.Volume = bIsSoundEnabled ? InitialVolume : 0.0f;
    }
}

void UCSoundWidget::OnSoundToggleChanged(bool bIsChecked)
{
    // üũ�ڽ� ���¿� ���� ���� Ŭ������ ������ ����
    if (SoundClass)
    {
        SoundClass->Properties.Volume = bIsChecked ? VolumeSlider->GetValue() : 0.0f;
    }
}

void UCSoundWidget::OnVolumeSliderChanged(float Value)
{
    // �����̴� ���� ���� ���� Ŭ������ ������ ����
    if (SoundClass && SoundToggleCheckBox->IsChecked())
    {
        SoundClass->Properties.Volume = Value;
    }
}