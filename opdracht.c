#include <stdio.h>
#include <stdlib.h>

#define __DEBUG

#define BMPINPUTFILE "puppy.bmp"

int main()
{
    #ifdef __DEBUG
        printf("DEBUG info: BMP transformer\n");
    #endif

    FILE* inputFilePointer = fopen(BMPINPUTFILE, "rb"); //maak een file pointer naar de afbeelding
    if(inputFilePointer == NULL) //Test of het open van de file gelukt is!
    {
        printf("Something went wrong while trying to open %s\n", BMPINPUTFILE);
        exit(EXIT_FAILURE);
    }

    #ifdef __DEBUG
        printf("DEBUG info: Opening File OK: %s\n", BMPINPUTFILE);
    #endif

    unsigned char bmpHeader[54]; // voorzie een array van 54-bytes voor de BMP Header
    fread(bmpHeader, sizeof(unsigned char), 54, inputFilePointer); // lees de 54-byte header

    //Informatie uit de header (wikipedia)
    // haal de hoogte en breedte uit de header
    int breedte = *(int*)&bmpHeader[18];
    int hoogte = *(int*)&bmpHeader[22];

    #ifdef __DEBUG
        printf("DEBUG info: breedte = %d\n", breedte);
        printf("DEBUG info: hoogte = %d\n", hoogte);
    #endif

    int imageSize = 3 * breedte * hoogte; //ieder pixel heeft 3 byte data: rood, groen en blauw (RGB)
    unsigned char* inputPixels = (unsigned char *) calloc(imageSize, sizeof(unsigned char)); // allocate een array voor alle pixels
	

	
    fread(inputPixels, sizeof(unsigned char), imageSize, inputFilePointer); // Lees alle pixels (de rest van de file
    fclose(inputFilePointer);
	for(int i =0; i < imageSize-2; i+=3)
	{
		printf("pixel %d: B= %d, G=%d, R=%d\n", i, inputPixels[i], inputPixels[i+1], inputPixels[i+2]);
	}
   
    fclose(inputFilePointer);
    free(inputPixels);
    
    return 0;
}
//grootte van bestand lezen
int size_of_image(FILE *fp1)
{
	int width, height;

	fseek(fp1, 0x12, SEEK_SET);	//breedte
	fread(&width, sizeof(int), 1, fp1);	//breedte
	fread(&height, sizeof(int), 1, fp1);	//hoogte
	printf("grootte van het bestand is: %d x %d \n", width, height);
	fseek(fp1, 0L, SEEK_SET);	//start positie

	return ((width * height * 3) / 8);
}
//invoegen van geheime text
void secret_text(FILE *fp2)
{
	char ch;
	while ((ch = getchar())!=EOF)
	{
		putc(ch,fp2);	
	}
}

//grootte van bericht
int secret_text_size(FILE *fp2)
{
	int size_txt;
	fseek(fp2, 0L, SEEK_END);
	size_txt = ftell(fp2);
	fseek(fp2, 0L, SEEK_SET);
	return size_txt;
}
//aantal bits
int get_bit(char byte, int bit)
{
	return ((byte >> 8 - bit) & 1);
}

//encoden van bericht
void stega_encrypt(FILE *fp1, FILE *fp2, FILE *fp3)
{
	char file_buff = 0, msg_buff = 0, ch;	
	int i;
	int bit_msg;
	while((msg_buff = fgetc(fp2)) != EOF)
	{
		for(i = 1; i <= 8; i++)
		{
			file_buff = fgetc(fp1);

			int file_byte_lsb = (file_buff & 1);

			bit_msg = get_bit(msg_buff, i);

			if(file_byte_lsb == bit_msg)
			{
				fputc(file_buff, fp3);
			}
			else
			{
				if(file_byte_lsb == 0)
					file_buff = (file_buff | 1);
				else
					file_buff = (file_buff ^ 1);

				fputc(file_buff, fp3);
			}
		}
	}

	
	while(!feof(fp1))
	{
		char tmp_cpy = fgetc(fp1);
		fputc(tmp_cpy,fp3);

	}

	if(msg_buff == EOF)
		printf("\n*** Succes ***\n");
	else
		printf("\n*** fail ***\n");
}
//Encoden van strings
void string_encrypt(char *str,FILE *fp1, FILE *fp3)
{

	char file_buff, msg_buff;
	int i, j = 0;
	int bit_msg;
	while((msg_buff = str[j]) != '\0')
	{
		for(i = 1; i <= 8; i++)
		{
			file_buff = fgetc(fp1);

			int file_byte_lsb = (file_buff & 1);

			bit_msg = get_bit(msg_buff, i);

			if(file_byte_lsb == bit_msg)
			{
				fputc(file_buff, fp3);
			}
			else
			{
				if(file_byte_lsb == 0)
					file_buff = (file_buff | 1);
				else
					file_buff = (file_buff ^ 1);

				fputc(file_buff, fp3);
			}
		}
		j++;
	}
}
//Encoden van nummers
void size_encrypt(int num,FILE *fp1, FILE *fp3)
{

	char file_buff;	
	int i, j = 0;
	int bit_msg;

	for(i = 1; i <= 8; i++)
	{
		file_buff = fgetc(fp1);

		int file_byte_lsb = (file_buff & 1);

		bit_msg = get_bit(num, i);

		if(file_byte_lsb == bit_msg)
		{
			fputc(file_buff, fp3);
		}
		else
		{
			if(file_byte_lsb == 0)
				file_buff = (file_buff | 1);
			else
				file_buff = (file_buff ^ 1);

			fputc(file_buff, fp3);
		}
	}

}

int Encoding(char *argv_2,char *argv_3,char *argv_4)
{
	FILE *fp1, *fp2, *fp3;

	//openen van file
	if((fp1 = fopen(argv_2, "r+")) == NULL) 
	{
		printf("kan file niet openen %s.\nAborting\n", argv_2);
		return 1;
	}

	int size_image = size_of_image(fp1);
	printf("Totaal %d Characters opgeslagen in %s.\n", size_image, argv_2);

	
	fp2 = fopen(argv_3, "w+");

	
	printf("typ text en ctrl+d om te stoppen : \t");
	secret_text(fp2);

	int size_txt = secret_text_size(fp2);
	printf("\nSize van het bestand is ==> %d\n", size_txt);

	
	if(size_image < size_txt)
	{
		printf("\n*** bericht groter dan bestand ***\n");
		return 1;
	}

	
	fp3 = fopen(argv_4, "w+");
	if (fp3== NULL) 
	{
		fprintf(stderr, "kan output niet creëren %s\n",argv_4);
		exit(1);
	}

	
	int i, c = 0;
	char tmp_cpy;
	rewind(fp1);	
	for(i = 0;i < 54; i++)
	{
		tmp_cpy = fgetc(fp1);
		fputc(tmp_cpy,fp3);
		c++;
	}

	
	if(i == c)
		printf("\n*** Succes ***\n");
	else
	{
		printf("\n*** Fail ***\n");
		return 0;
	}

	

	//Encoden van bericht
	size_encrypt(size_txt,fp1,fp3);
	stega_encrypt(fp1,fp2,fp3);

	// afsluiten
	fclose(fp1);	
	fclose(fp2);
	fclose(fp3);

	return 0;

}
// decoden van de grootte
void size_decryption(FILE *pf1, int *size_txt)
{
	int file_buff = 0, i;
	int ch, bit_msg;
	for (i = 0; i < 8; i++)
	{
		ch = fgetc(pf1);
		bit_msg = (ch & 1);
		if (bit_msg)
		{
			file_buff = (file_buff << 1) | 1;
		}
		else
		{
			file_buff = file_buff << 1;
		}
	}
	*size_txt = file_buff;
}

// decoden van de strings
void string_decryption(FILE *pf1,char *strng,int size)
{
	int file_buff=0, i, j=0, k=0;
	int ch, bit_msg;
	for (i = 0; i < (size * 8); i++)
	{
		j++;
		ch = fgetc(pf1);
		bit_msg = (ch & 1);
		if (bit_msg)
		{
			file_buff = (file_buff << 1) | 1;
		}
		else
		{
			file_buff = file_buff << 1;
		}

		if ( j == 8)
		{
			strng[k] =(char)file_buff; 
			j=0;
			k++;
			file_buff = 0;
		}
	}
	strng[k] = '\0';
}
// decoden van de text
void secret_decryption(int size_txt, FILE *pf1, FILE *pf2)
{
	int file_buff=0, i, j = 0, k = 0;
	int ch,bit_msg;
	char output[250] = {0};
	for (i = 0; i < (size_txt * 8); i++)
	{
		j++;
		ch = fgetc(pf1);
		bit_msg = (ch & 1);
		if (bit_msg)
		{
			file_buff = (file_buff << 1) | 1;
		}
		else
		{
			file_buff = file_buff << 1;
		}

		if ( j == 8)
		{
			putc(file_buff, pf2);
			output[k++] = file_buff;
			j=0;
			file_buff = 0;
		}
	}
	printf("\n*** geheime text is ==> %s\n\n", output);
}
int Decode(char *argv_2, char *argv_4)
{
	FILE *pf1, *pf2;
	
	int size, size1, size_txt;

	//opening Image File
	if((pf1 = fopen(argv_2, "r")) == NULL) 
	{
		printf("kon file niet openen %s.\nAborting\n", argv_2);
		return 1;
	}

	fseek(pf1,54 ,SEEK_SET);
	
	if((pf2 = fopen(argv_4, "w+")) == NULL) 
	{
		printf("kon file niet openen %s.\nAborting\n", argv_4);
		return 1;
	}

	


	// geheime text
	size_decryption(pf1, &size_txt);
	secret_decryption(size_txt, pf1, pf2);

	printf("*** de geheime text is gekopiëerd naar ==> %s\n\n", argv_4);

	// afsluiten
	fclose(pf1);

}
