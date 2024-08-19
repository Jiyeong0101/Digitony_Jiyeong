#include "AlogorithmQuestion/CAlgorithmQuestion.h"
#include "Engine/StaticMeshActor.h"
#include "Containers/Array.h"
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
//    UE_LOG(LogTemp, Log, TEXT("ACAlgorithmQuestion::BeginPlay() ȣ���"));
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

    UE_LOG(LogTemp, Log, TEXT("�� ������ �ε� �Ϸ�: Width=%d, Height=%d, Depth=%d"), Width, Height, Depth);

    // �� ���� ������ �񵿱������� ó��
    AsyncTask(ENamedThreads::GameThread, [this]()
        {
            CreateMap();
        });
}

// Create Map
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

                // �̵����� Movable�� ����
                NewMesh->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                NewMesh->GetStaticMeshComponent()->SetWorldScale3D(FVector(0.1f));

                // ������ ���͸� �迭�� �߰�
                CreatedActors.Add(NewMesh);

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
                        UE_LOG(LogTemp, Log, TEXT("StartBlock ��ġ: %s"), *StartBlockLocation.ToString());
                    }
                    break;

                case '3':
                    if (EndBlock)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(EndBlock);
                    }
                    break;

                case '4':
                    if (TransparentObstacleBlock)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(TransparentObstacleBlock);
                    }
                    break;

                case '5':
                    if (CoinMesh)
                    {
                        NewMesh->GetStaticMeshComponent()->SetStaticMesh(CoinMesh);
                        CreatedCoins.Add(NewMesh);  // ������ �迭�� �߰�
                        UE_LOG(LogTemp, Log, TEXT("���� ��ȯ: ��ġ = %s"), *NewLocation.ToString());
                    }
                    break;

                default:
                    UE_LOG(LogTemp, Warning, TEXT("��ȿ���� ���� �� ������: %c"), MapValue);
                    break;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CreateMap() �Ϸ��"));
}


// Start Magic
void ACAlgorithmQuestion::StartMagic()
{
    if (!MagicCircle)
    {
        UE_LOG(LogTemp, Warning, TEXT("MagicCircle�� �������� �ʾҽ��ϴ�."));
        return;
    }

    // Luni�� StartBlock�� ��ġ�� �̵��ϰ� �ʱ� ȸ���� ����
    if (Luni && !StartBlockLocation.IsZero())
    {
        Luni->SetActorLocation(StartBlockLocation);
        Luni->SetActorRotation(InitialLuniRotation);  // �ʱ� ȸ�� ����
        UE_LOG(LogTemp, Log, TEXT("Luni�� ���� ��ġ�� �̵��Ǿ����� �ʱ� ȸ���� �����Ǿ����ϴ�."));
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
        UE_LOG(LogTemp, Log, TEXT("�ڵ� ��� ����: %d"), CurrentCodeBlockIndex);
        MoveLuni(MagicCircle->CodeBlocks[CurrentCodeBlockIndex].CodeBlockType);
        CurrentCodeBlockIndex++;
    }
    else
    {
        // ��� �ڵ� ��� ���� �Ϸ� �� Ÿ�̸� ����
        GetWorldTimerManager().ClearTimer(CodeBlockTimerHandle);
        //UE_LOG(LogTemp, Log, TEXT("��� �ڵ� ��� ���� �Ϸ�"));
    }
}

// Move Luni
void ACAlgorithmQuestion::MoveLuni(ECodeBlockType InCodeBlockType)
{
    static int32 RepeatCount = 0;
    static int32 RepeatStartIndex = 0;
    static bool bInRepetition = false;

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
        UE_LOG(LogTemp, Log, TEXT("���� ��: NewLocation = %s"), *NewLocation.ToString());
        break;

    case ECodeBlockType::RightTurn:
        Luni->AddActorLocalRotation(FRotator(0.f, 90.f, 0.f));
        UE_LOG(LogTemp, Log, TEXT("��ȸ�� ��"));
        break;

    case ECodeBlockType::LeftTurn:
        Luni->AddActorLocalRotation(FRotator(0.f, -90.f, 0.f));
        UE_LOG(LogTemp, Log, TEXT("��ȸ�� ��"));
        break;

    case ECodeBlockType::Jump:
        NewLocation += FVector(0.f, 0.f, Spacing);  // ���� ����
        NewLocation += Luni->GetActorForwardVector() * Spacing;  // ������ �̵�
        UE_LOG(LogTemp, Log, TEXT("���� ��: NewLocation = %s"), *NewLocation.ToString());
        break;

    case ECodeBlockType::DownJump:
        NewLocation += Luni->GetActorForwardVector() * Spacing;  // ������ �̵�
        NewLocation -= FVector(0.f, 0.f, Spacing);  // �Ʒ��� ����
        UE_LOG(LogTemp, Log, TEXT("�ϰ� ��: NewLocation = %s"), *NewLocation.ToString());
        break;

    case ECodeBlockType::Repetition:
        RepeatStartIndex = CurrentCodeBlockIndex;
        bInRepetition = true;
        break;

    case ECodeBlockType::Number_2:
    case ECodeBlockType::Number_3:
    case ECodeBlockType::Number_4:
        if (bInRepetition)
        {
            RepeatCount = static_cast<int32>(InCodeBlockType) - static_cast<int32>(ECodeBlockType::Number_2) + 2;
            bInRepetition = false;
            UE_LOG(LogTemp, Log, TEXT("�ݺ� Ƚ�� ����: %d"), RepeatCount);
            
            // Repetition ��� ó��
            if (!bInRepetition && RepeatCount > 0)
            {
                if (CurrentCodeBlockIndex >= MagicCircle->CodeBlocks.Num() - 1)
                {
                    RepeatCount--;
                    CurrentCodeBlockIndex = RepeatStartIndex;

                    if (RepeatCount <= 0)
                    {
                        CurrentCodeBlockIndex++;
                        return;
                    }
                }
            }
        }
        break;

    default:
        UE_LOG(LogTemp, Warning, TEXT("��ȿ���� ���� �ڵ� ��� Ÿ���Դϴ�."));
        return;
    }

    

    // �浹 �˻�
    FCollisionQueryParams CollisionParams;
    const float CollisionRadius = Spacing * 0.4f;
    TArray<FOverlapResult> Overlaps;

    if (GetWorld()->OverlapMultiByChannel(Overlaps, NewLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CollisionRadius), CollisionParams))
    {
        for (auto& Overlap : Overlaps)
        {
            AStaticMeshActor* HitActor = Cast<AStaticMeshActor>(Overlap.GetActor());

            if (HitActor)
            {
                UStaticMesh* HitMesh = HitActor->GetStaticMeshComponent()->GetStaticMesh();

                if (HitMesh == CoinMesh)
                {
                    UE_LOG(LogTemp, Log, TEXT("��ϰ� ������ �Ծ����ϴ�."));
                    CreatedCoins.Remove(HitActor);
                    HitActor->Destroy();
                    return;
                }

                if (HitMesh == ObstacleBlock || HitMesh == TransparentObstacleBlock)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Luni�� ��ֹ��� �浹�߽��ϴ�."));
                    return;
                }
                else if (HitMesh == EndBlock)
                {
                    if (CreatedCoins.Num() > 0)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("��� ������ �Ա� ������ ���� Ŭ������ �� �����ϴ�."));
                        return;
                    }

                    UE_LOG(LogTemp, Log, TEXT("Luni�� EndBlock�� �����߽��ϴ�."));
                    FinishMagic();
                    return;
                }
            }
        }
    }

    // �浹�� ���ٸ� ��ġ ������Ʈ
    Luni->SetActorLocation(NewLocation);
    UE_LOG(LogTemp, Log, TEXT("Luni�� �� ��ġ: %s"), *NewLocation.ToString());
}






// Clear Map
void ACAlgorithmQuestion::ClearMap()
{
    // Clear the array of created actors
    for (AStaticMeshActor* Actor : CreatedActors)
    {
        if (Actor) // Check if the actor is valid
        {
            Actor->Destroy(); // Mark the actor for deletion
        }
    }
    CreatedActors.Empty(); // Clear the array to prevent further access

    // Destroy Luni
    if (Luni)
    {
        Luni->Destroy();
        Luni = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("���� Ŭ����Ǿ����ϴ�."));

}
