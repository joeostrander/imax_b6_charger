<#
  Joe Ostrander
  Monitor SkyRC imax charger and notify when charging complete
  2018.03.16
  

#>

# -------------------------------------------------------
#Note!  Set these to your settings.  Verify server, port, etc.
$comPort = "COM##"
$smtpServer = "smtp.gmail.com"
$EmailPort = 587
$EmailTo = "<your email address>"
$EmailFrom = "<your email address>"
$EmailUsername = "<your email username>"
$EmailPassword = "<your email password>"
$EmailSubject = "Battery charge done!"
# -------------------------------------------------------

$port= new-Object System.IO.Ports.SerialPort $comPort,57600,None,8,one
$port.Open()

$boolContinue = $true
while ($boolContinue)
{
  $existing = $port.ReadExisting()

  if ($existing)
  { 
    if ($existing.Contains("NO RESPONSE"))
    {
      $boolContinue = $false
      break;
    }
    else
    {


      $last_message += $existing
      $last_message = $last_message.Substring($last_message.LastIndexOf("Status:"))
      Clear-Host
      Write-Host $last_message #-NoNewLine

    }
    $existing = ""
  }
}

$port.Close()

Write-Host ""
Write-Host "DONE!"


$EmailBody = $last_message


$msg = New-Object Net.Mail.MailMessage
$msg.From = $EmailFrom
$msg.Sender = $EmailFrom  #important if the sender is fake!
$msg.To.Add($EmailTo)
$msg.Subject = $EmailSubject
$msg.Body = $EmailBody

$smtp = New-Object Net.Mail.SmtpClient($smtpServer)
$smtp.Credentials = New-Object System.Net.NetworkCredential($EmailUsername,$EmailPassword)
$smtp.EnableSsl = $true
$smtp.Port = $EmailPort
$smtp.Send($msg)