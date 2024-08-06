#include "AlogorithmQuestion/CAlgorithmQuestion.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"

// ������
ACAlgorithmQuestion::ACAlgorithmQuestion()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentCodeBlockIndex = 0; // �ε��� �ʱ�ȭ
}

// Begin Play
void ACAlgorithmQuestion::BeginPlay()
{
    Super::BeginPlay();
    LoadMapData();
}

// Tick �Լ�
void ACAlgorithmQuestion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Load Map Data
void ACAlgorithmQuestion::LoadMapData()
{
    if (!MapData.DataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapData DataTable�� �������� �ʾҽ��ϴ�."));
        return;
    }

    FAlgorithmQuestionMapData* Row = MapData.DataTable->FindRow<FAlgorithmQuestionMapData>(MapData.RowName, TEXT(""));

    if (!Row)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapData ���� ã�� �� �����ϴ�."));
        return;
    }

    ID = Row->ID;
    Width = Row->Width;
    Height = Row->Height;
    Depth = Row->Depth;
    Map = Row->Map;

    CreateMap();
}

// Create Map
void ACAlgorithmQuestion::CreateMap()
{
    StartBlockLocation = FVector::ZeroVector; // ���� ��ġ �ʱ�ȭ

    for (int32 Z = 0; Z < Depth; Z++)
    {
        for (int32 Y = 0; Y < Height; Y++)
        {
            for (int32 X = 0; X < Width; X++)
            {
                FVector ActorLocation = this->GetActorLocation();
                FVector NewLocation = FVector(ActorLocation.X + (X * Spacing), ActorLocation.Y + (Y * Spacing), ActorLocation.Z + (Z * Spacing));
                FRotator NewRotator = this->GetActorRotation();

                int32 Index = Z * (Width * Height) + Y * Width + X;

                if (!Map.IsValidIndex(Index))
                {
                    UE_LOG(LogTemp, Warning, TEXT("�� �ε����� ��ȿ���� �ʽ��ϴ�: %d"), Index);
                    continue;
                }

                char MapValue = Map[Index];

                if (MapValue == '0')
                {
                    continue;
                }

                AStaticMeshActor* NewMesh = GetWorld()->SpawnActor<AStaticMeshActor>(NewLocation, NewRotator);
                if (!NewMesh || !NewMesh->GetStaticMeshComponent())
                {
                    UE_LOG(LogTemp, Error, TEXT("�� �޽� ���� ������ �����߽��ϴ�."));
                    continue;
                }

                CreatedActors.Add(NewMesh); // ������ ���͸� �迭�� �߰�

                switch (MapValue)
                {
                case '1':
                    if (ObstacleBlock)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(ObstacleBlock);
                    }
                    break;

                case '2':
                    if (StartBlock)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(StartBlock);
                        StartBlockLocation = NewLocation; // ���� ��ġ ����
                    }
                    break;

                case '3':
                    if (EndBlock)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(EndBlock);
                    }
                    break;

                default:
                    UE_LOG(LogTemp, Warning, TEXT("��ȿ���� ���� �� ������: %c"), MapValue);
                    break;
                }
            }
        }
    }
}

// Start Magic
void ACAlgorithmQuestion::StartMagic()
{
    if (!MagicCircle)
    {
        UE_LOG(LogTemp, Warning, TEXT("MagicCircle�� �������� �ʾҽ��ϴ�."));
        return;
    }

    // Luni�� StartBlock�� ��ġ�� �̵�
    if (Luni && !StartBlockLocation.IsZero())
    {
        Luni->SetActorLocation(StartBlockLocation);
        UE_LOG(LogTemp, Log, TEXT("Luni�� ���� ��ġ�� �̵��Ǿ����ϴ�."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("StartBlock�� ��ġ�� �������� �ʾҽ��ϴ�."));
    }

    // �ڵ� ��� �ε��� �ʱ�ȭ
    CurrentCodeBlockIndex = 0;

    // Ÿ�̸� ���� (TimerInterval�� ����Ͽ� Ÿ�̸� ���� ����)
    GetWorldTimerManager().SetTimer(CodeBlockTimerHandle, this, &ACAlgorithmQuestion::ExecuteCodeBlock, TimerInterval, true);
}

// Execute Code Block
void ACAlgorithmQuestion::ExecuteCodeBlock()
{
    if (CurrentCodeBlockIndex < MagicCircle->CodeBlocks.Num())
    {
        MoveLuni(MagicCircle->CodeBlocks[CurrentCodeBlockIndex].CodeBlockType);
        CurrentCodeBlockIndex++;
    }
    else
    {
        // ��� �ڵ� ��� ���� �Ϸ� �� Ÿ�̸� ����
        GetWorldTimerManager().ClearTimer(CodeBlockTimerHandle);
        // UE_LOG(LogTemp, Log, TEXT("��� �ڵ� ��� ���� �Ϸ�"));
    }
}

// Move Luni
void ACAlgorithmQuestion::MoveLuni(ECodeBlockType InCodeBlockType)
{
    if (!Luni)
    {
        UE_LOG(LogTemp, Warning, TEXT("Luni�� �������� �ʾҽ��ϴ�."));
        return;
    }

    FVector NewLocation = Luni->GetActorLocation();

    switch (InCodeBlockType)
    {
    case ECodeBlockType::Forward:
        NewLocation += Luni->GetActorForwardVector() * Spacing;
        break;
    case ECodeBlockType::RightTurn:
        Luni->AddActorLocalRotation(FRotator(0.f, 90.f, 0.f));
        break;
    case ECodeBlockType::LeftTurn:
        Luni->AddActorLocalRotation(FRotator(0.f, -90.f, 0.f));
        break;
    case ECodeBlockType::Jump:
        NewLocation += FVector(0.f, 0.f, Spacing);
        NewLocation += Luni->GetActorForwardVector() * Spacing;
        break;
    case ECodeBlockType::Repetition:
        // �ݺ� ���� ����
        break;
    case ECodeBlockType::Number:
        // ���� ���� ����
        break;
    default:
        break;
    }

    // ��ֹ� �� EndBlock ������ �˻�
    FCollisionQueryParams CollisionParams;
    if (GetWorld()->OverlapBlockingTestByChannel(NewLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(50.f), CollisionParams))
    {
        // HitResult�� �ʿ� ����
        TArray<FOverlapResult> Overlaps;
        if (GetWorld()->OverlapMultiByChannel(Overlaps, NewLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(50.f), CollisionParams))
        {
            for (auto& Overlap : Overlaps)
            {
                AStaticMeshActor* HitActor = Cast<AStaticMeshActor>(Overlap.GetActor());

                if (HitActor)
                {
                    UStaticMesh* HitMesh = HitActor->GetStaticMeshComponent()->GetStaticMesh();

                    if (HitMesh == ObstacleBlock)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Luni�� ��ֹ��� ���ƽ��ϴ�."));
                        return;
                    }
                    else if (HitMesh == EndBlock)
                    {
                        UE_LOG(LogTemp, Log, TEXT("Luni�� EndBlock�� �����߽��ϴ�."));
                        
                        FinishMagic();
                        return;
                    }
                }
            }
        }
    }

    Luni->SetActorLocation(NewLocation);
}

// Clear Map
void ACAlgorithmQuestion::ClearMap()
{
    // ������ ��� ���� ����
    for (AStaticMeshActor* Actor : CreatedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }
    CreatedActors.Empty();

    // Luni ����
    if (Luni)
    {
        Luni->Destroy();
        Luni = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("���� Ŭ����Ǿ����ϴ�."));
}
