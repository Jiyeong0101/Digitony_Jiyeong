#include "MagicClicle/CMagicCircle.h"
#include "Components/BoxComponent.h"
#include "CodeBlocks/CCodeBlockBase.h"


ACMagicCircle::ACMagicCircle()
{
	PrimaryActorTick.bCanEverTick = true;

	MagicCircleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagicCircleMeshComponent"));
	BoxCollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollisionComponent"));

	SetRootComponent(MagicCircleMeshComponent);
	BoxCollisionComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void ACMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACMagicCircle::AddCodeBlock(ACCodeBlockBase* InCodeBlock)
{
	if (InCodeBlock == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("InCodeBlock is null"));
		return;
	}

	// �ڵ� ����� 16�� �̻� �Էµ��� ���ϰ� ���� ó��
	if (CodeBlocks.Num() >= CodeBlockArrayNum)
	{
		UE_LOG(LogTemp, Warning, TEXT("CodeBlock is full"));
		return;
	}

	CodeBlocks.Add(InCodeBlock);

	UpdateMagicCircle();
}

void ACMagicCircle::RomoveCodeBlock()
{
	// �ڵ� ����� 16�� �̻� �Էµ��� ���ϰ� ���� ó��
	if (0 >= CodeBlocks.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("CodeBlock is none"));
		return;
	}

	CodeBlocks.Pop();

	UpdateMagicCircle();
}

